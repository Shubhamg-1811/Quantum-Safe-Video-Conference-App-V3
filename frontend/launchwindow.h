#ifndef LAUNCHWINDOW_H
#define LAUNCHWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QComboBox>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QProcess>

class LaunchWindow : public QMainWindow
{
    Q_OBJECT

public:
    LaunchWindow(QWidget *parent = nullptr);
    ~LaunchWindow();

private slots:
    void onModeChanged();
    void onRefreshIpClicked();
    void onConnectClicked();
    void updateStatusMessage(const QString &message, const QString &type);

    // Backend slots - ADD THESE
    void onBackendStarted();
    void onBackendOutput();
    void onBackendError();
    void onBackendFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    // UI Elements
    QRadioButton *serverRadio;
    QRadioButton *clientRadio;
    // QLineEdit *myIpEdit;
    QLineEdit *peerIpEdit;
    QComboBox *myIpCombo;
    QLineEdit *usernameEdit;
    QLabel *usernameLabel;
    QLabel *usernameHelp;
    QPushButton *connectBtn;
    QLabel *statusLabel;

    QProcess *backendProcess;

    // Helper functions
    QStringList getAllLocalIPs();
    void populateIpDropdown();
    bool validateIP(const QString &ip);
    void setupUI();
};

#endif // LAUNCHWINDOW_H
