#include <gst/gst.h>
#include <iostream>
#include <glib.h>
#include "auth_protocol.h"

using namespace std;

// Create GstBuffer from key vector
static GstBuffer* make_key_buffer(const std::vector<uint8_t>& key_vec) {
    GstBuffer *key_buf = gst_buffer_new_allocate(NULL, key_vec.size(), NULL);
    gst_buffer_fill(key_buf, 0, key_vec.data(), key_vec.size());
    return key_buf;
}

// Signal handler for srtpdec request-key
static GstCaps* on_request_key(GstElement *srtpdec, guint ssrc, gpointer user_data) {
    cout << "Key requested for SSRC: " << ssrc << endl;
    
    GstBuffer *key_buf = make_key_buffer(SRTP_KEY);
    
    GstCaps *caps = gst_caps_new_simple("application/x-srtp",
        "srtp-key", GST_TYPE_BUFFER, key_buf,
        "srtp-cipher", G_TYPE_STRING, "aes-256-icm",
        "srtcp-cipher", G_TYPE_STRING, "aes-256-icm",
        "srtp-auth", G_TYPE_STRING, "hmac-sha1-80",
        "srtcp-auth", G_TYPE_STRING, "hmac-sha1-80",
        NULL);
    
    gst_buffer_unref(key_buf);
    return caps;
}

// Configure jitterbuffer for low latency
static void on_new_jitterbuffer(GstElement *rtpbin, GstElement *jitterbuffer, 
                                guint session, guint ssrc, gpointer user_data) {
    g_object_set(jitterbuffer,
        "latency", 50,
        "drop-on-latency", TRUE,
        "do-lost", FALSE,
        "do-retransmission", FALSE,
        "rtx-delay", 20,
        NULL);
}

// Bus message handler
static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
    GMainLoop *loop = (GMainLoop *)data;

    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS:
            cout << "End of stream" << endl;
            g_main_loop_quit(loop);
            break;

        case GST_MESSAGE_ERROR: {
            gchar *debug;
            GError *error;
            gst_message_parse_error(msg, &error, &debug);
            cerr << "Error: " << error->message << endl;
            g_free(debug);
            g_error_free(error);
            g_main_loop_quit(loop);
            break;
        }

        default:
            break;
    }

    return TRUE;
}

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    if (argc != 3) {
        cout << "Usage: " << argv[0] << " <server_ip> <username>" << endl;
        return -1;
    }

    const char* server_ip = argv[1];
    string username = argv[2];

    // Perform authenticated key exchange BEFORE creating pipeline
    if (!client_perform_authenticated_key_exchange(server_ip, 9000, username)) {
        cerr << "Authenticated key exchange failed!" << endl;
        return -1;
    }

    cout << "\n=== Starting Secure Video/Audio Streaming ===" << endl;
    cout << "Logged in as: " << username << "\n" << endl;

    // GStreamer pipeline
    string pipeline_desc = 
        // Send video
        "autovideosrc ! videoconvert ! video/x-raw,format=I420 ! "
        "x264enc tune=zerolatency bitrate=500 speed-preset=superfast key-int-max=30 bframes=0 aud=false "
        "byte-stream=true sliced-threads=true rc-lookahead=0 sync-lookahead=0 ! "
        "rtph264pay config-interval=1 pt=96 mtu=1400 ! rtpbin_send.send_rtp_sink_0 "
        
        "rtpbin_send.send_rtp_src_0 ! srtpenc name=video_send_encrypt "
        "rtp-cipher=aes-256-icm rtcp-cipher=aes-256-icm rtp-auth=hmac-sha1-80 rtcp-auth=hmac-sha1-80 ! "
        "udpsink host=" + string(server_ip) + " port=5000 sync=false async=false "
        
        "rtpbin_send.send_rtcp_src_0 ! srtpenc name=video_rtcp_enc "
        "rtp-cipher=aes-256-icm rtcp-cipher=aes-256-icm rtp-auth=hmac-sha1-80 rtcp-auth=hmac-sha1-80 ! "
        "udpsink host=" + string(server_ip) + " port=5001 sync=false async=false "
        
        "udpsrc port=5005 buffer-size=212992 ! srtpdec name=video_rtcp_recv_dec ! rtpbin_send.recv_rtcp_sink_0 "
        
        // Send audio
        "autoaudiosrc ! audioconvert ! audioresample ! opusenc bitrate=64000 ! "
        "rtpopuspay pt=97 mtu=1400 ! rtpbin_send.send_rtp_sink_1 "
        
        "rtpbin_send.send_rtp_src_1 ! srtpenc name=audio_send_encrypt "
        "rtp-cipher=aes-256-icm rtcp-cipher=aes-256-icm rtp-auth=hmac-sha1-80 rtcp-auth=hmac-sha1-80 ! "
        "udpsink host=" + string(server_ip) + " port=5002 sync=false async=false "
        
        "rtpbin_send.send_rtcp_src_1 ! srtpenc name=audio_rtcp_enc "
        "rtp-cipher=aes-256-icm rtcp-cipher=aes-256-icm rtp-auth=hmac-sha1-80 rtcp-auth=hmac-sha1-80 ! "
        "udpsink host=" + string(server_ip) + " port=5003 sync=false async=false "
        
        "udpsrc port=5007 buffer-size=212992 ! srtpdec name=audio_rtcp_recv_dec ! rtpbin_send.recv_rtcp_sink_1 "
        
        // Receive video
        "udpsrc port=5010 buffer-size=212992 ! srtpdec name=video_dec ! "
        "application/x-rtp,media=(string)video,clock-rate=(int)90000,encoding-name=(string)H264,payload=(int)96 ! "
        "rtpbin_recv.recv_rtp_sink_0 "
        
        "rtpbin_recv. ! rtph264depay ! avdec_h264 ! videoconvert ! autovideosink sync=false "
        
        "udpsrc port=5011 buffer-size=212992 ! srtpdec name=video_rtcp_dec ! rtpbin_recv.recv_rtcp_sink_0 "
        
        // Receive audio
        "udpsrc port=5012 buffer-size=212992 ! srtpdec name=audio_dec ! "
        "application/x-rtp,media=(string)audio,clock-rate=(int)48000,encoding-name=(string)OPUS,payload=(int)97 ! "
        "rtpbin_recv.recv_rtp_sink_1 "
        
        "rtpbin_recv. ! rtpopusdepay ! opusdec ! audioconvert ! audioresample ! autoaudiosink sync=false "
        
        "udpsrc port=5013 buffer-size=212992 ! srtpdec name=audio_rtcp_dec ! rtpbin_recv.recv_rtcp_sink_1 "
        
        "rtpbin name=rtpbin_recv latency=50 drop-on-latency=true do-retransmission=false "
        "rtpbin name=rtpbin_send latency=50 drop-on-latency=true do-retransmission=false";

    GError *error = NULL;
    GstElement *pipeline = gst_parse_launch(pipeline_desc.c_str(), &error);
    if (error) {
        cerr << "Pipeline parse error: " << error->message << endl;
        g_error_free(error);
        return -1;
    }

    // Get rtpbin elements and connect jitterbuffer signal
    GstElement *rtpbin_recv = gst_bin_get_by_name(GST_BIN(pipeline), "rtpbin_recv");
    GstElement *rtpbin_send = gst_bin_get_by_name(GST_BIN(pipeline), "rtpbin_send");

    if (rtpbin_recv) {
        g_signal_connect(rtpbin_recv, "new-jitterbuffer", G_CALLBACK(on_new_jitterbuffer), NULL);
        gst_object_unref(rtpbin_recv);
    }

    if (rtpbin_send) {
        g_signal_connect(rtpbin_send, "new-jitterbuffer", G_CALLBACK(on_new_jitterbuffer), NULL);
        gst_object_unref(rtpbin_send);
    }

    // Set keys for all srtpenc elements
    const char* enc_names[] = {"video_send_encrypt", "audio_send_encrypt", "video_rtcp_enc", "audio_rtcp_enc"};
    for (const char* name : enc_names) {
        GstElement *enc = gst_bin_get_by_name(GST_BIN(pipeline), name);
        if (enc) {
            GstBuffer *key_buf = make_key_buffer(SRTP_KEY);
            g_object_set(enc, "key", key_buf, NULL);
            gst_buffer_unref(key_buf);
            gst_object_unref(enc);
        }
    }

    // Set key request handler for all srtpdec elements
    const char* dec_names[] = {"video_dec", "audio_dec", "video_rtcp_dec", 
                               "audio_rtcp_dec", "video_rtcp_recv_dec", "audio_rtcp_recv_dec"};
    for (const char* name : dec_names) {
        GstElement *dec = gst_bin_get_by_name(GST_BIN(pipeline), name);
        if (dec) {
            g_signal_connect(dec, "request-key", G_CALLBACK(on_request_key), NULL);
            gst_object_unref(dec);
        }
    }

    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    GstBus *bus = gst_element_get_bus(pipeline);
    guint bus_watch_id = gst_bus_add_watch(bus, bus_call, loop);
    gst_object_unref(bus);

    cout << "Setting pipeline to PLAYING state..." << endl;
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    g_main_loop_run(loop);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_source_remove(bus_watch_id);
    g_main_loop_unref(loop);

    return 0;
}
