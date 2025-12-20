#include "launchwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QNetworkInterface>
#include <QRegularExpression>
#include <QProcess>
#include <QCoreApplication>
#include <QDebug>

LaunchWindow::LaunchWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Secure Video Conference");
    setFixedSize(550, 800);
    // setMinimumSize(500, 700);
    // setMaximumSize(500, 800);

    backendProcess = new QProcess(this);

    setupUI();
}

LaunchWindow::~LaunchWindow()
{
}

void LaunchWindow::setupUI()
{
    // Create central widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    centralWidget->setStyleSheet("background-color: #f5f5f5;");

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(40, 40, 40, 40);

    // ========== HEADER SECTION ==========
    QLabel *titleLabel = new QLabel("🔒 Quantum Secure Video Call", this);
    titleLabel->setStyleSheet(
        "font-size: 24px; "
        "font-weight: bold; "
        "color: #2c3e50; "
        "padding: 10px;"
        );
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    QLabel *subtitleLabel = new QLabel("Kyber768 + Dilithium + AES-256 Encryption", this);
    subtitleLabel->setStyleSheet(
        "font-size: 12px; "
        "color: #7f8c8d; "
        "padding-bottom: 10px;"
        );
    subtitleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(subtitleLabel);

    // ========== MODE SELECTION CARD ==========
    QGroupBox *modeGroup = new QGroupBox("Connection Mode", this);
    modeGroup->setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold; "
        "    font-size: 14px; "
        "    padding: 20px; "
        "    border: 2px solid #bdc3c7; "
        "    border-radius: 10px; "
        "    margin-top: 10px; "
        "    background-color: white;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin; "
        "    left: 15px; "
        "    padding: 0 5px;"
        "    color: #34495e;"
        "}"
        );

    QHBoxLayout *modeLayout = new QHBoxLayout(modeGroup);
    modeLayout->setSpacing(20);

    serverRadio = new QRadioButton("Host Meeting", this);
    clientRadio = new QRadioButton("Join Meeting", this);
    serverRadio->setChecked(true);

    QString radioStyle =
        "QRadioButton {"
        "    font-size: 15px; "
        "    font-weight: bold;"              // ADDED: Bold text
        "    padding: 10px; "
        "    spacing: 10px;"
        "    color: #2c3e50;"
        "    background-color: transparent;"
        "}"
        "QRadioButton::indicator {"
        "    width: 18px; "
        "    height: 18px;"
        "    border-radius: 9px;"            // ADDED: Makes it circular
        "    border: 2px solid #3498db;"     // ADDED: Blue border
        "    background-color: white;"       // ADDED: White background
        "}"
        "QRadioButton::indicator:checked {"  // ADDED: Style when selected
        "    background-color: #3498db;"     // Blue fill when checked
        "    border: 2px solid #2980b9;"     // Darker blue border
        "}";


    serverRadio->setStyleSheet(radioStyle);
    clientRadio->setStyleSheet(radioStyle);

    modeLayout->addStretch();
    modeLayout->addWidget(serverRadio);
    modeLayout->addWidget(clientRadio);
    modeLayout->addStretch();
    mainLayout->addWidget(modeGroup);

    // ========== NETWORK CONFIGURATION CARD ==========
    QGroupBox *networkGroup = new QGroupBox("Network Configuration", this);
    networkGroup->setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold; "
        "    font-size: 14px; "
        "    padding: 20px; "
        "    border: 2px solid #bdc3c7; "
        "    border-radius: 10px; "
        "    margin-top: 10px; "
        "    background-color: white;"
        "    color: #34495e;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin; "
        "    left: 15px; "
        "    padding: 0 5px;"
        "}"
        );

    QVBoxLayout *networkLayout = new QVBoxLayout(networkGroup);
    networkLayout->setSpacing(10);

    // Your IP Address
    QLabel *myIpLabel = new QLabel("Your IP Address:", this);
    myIpLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #34495e; background-color: transparent;");
    networkLayout->addWidget(myIpLabel);

    QHBoxLayout *myIpLayout = new QHBoxLayout();


    myIpCombo = new QComboBox(this);
    myIpCombo->setStyleSheet(
        "QComboBox {"
        "  background-color: #ecf0f1; "
        "  padding: 8px 12px; "
        "  padding-right: 30px;"
        "  border: 1px solid #bdc3c7; "
        "  border-radius: 5px; "
        "  font-size: 14px;"
        "  color: #2c3e50;"                    // Combo text color
        "}"
        "QComboBox:hover {"
        "  border: 1px solid #3498db;"
        "}"
        "QComboBox::drop-down {"
        "  subcontrol-origin: padding;"
        "  subcontrol-position: top right;"
        "  width: 30px;"
        "  border-left: 1px solid #bdc3c7;"
        "  border-top-right-radius: 5px;"
        "  border-bottom-right-radius: 5px;"
        "  background-color: #d5dbdb;"
        "}"
        "QComboBox::drop-down:hover {"
        "  background-color: #3498db;"
        "}"
        "QComboBox::down-arrow {"
        "  image: url(:/qt-project.org/styles/commonstyle/images/down-128.png);"
        "  width: 14px;"
        "  height: 12px;"
        "  background: transparent;"
        "  border: none;"
        "}"
        "QComboBox::down-arrow:hover {"
        "  background: transparent;"
        "  border: none;"
        "}"
        "QComboBox QAbstractItemView {"
        "  background-color: #ffffff;"         // White background
        "  color: #2c3e50;"                    // Dark text
        "  border: 2px solid #3498db;"         // Blue border
        "  selection-background-color: #3498db;"
        "  selection-color: #ffffff;"
        "  padding: 5px;"
        "  outline: none;"
        "}"
        "QComboBox QAbstractItemView::item {"
        "  padding: 8px 12px;"
        "  color: #2c3e50;"                    // Dark text for items
        "  background-color: #ffffff;"         // White background for items
        "  border: none;"
        "}"
        "QComboBox QAbstractItemView::item:selected {"
        "  background-color: #3498db;"         // Blue when selected
        "  color: #ffffff;"                    // White text when selected
        "}"
        "QComboBox QAbstractItemView::item:hover {"
        "  background-color: #ecf0f1;"         // Light grey on hover
        "  color: #2c3e50;"                    // Dark text on hover
        "}"
        );

    populateIpDropdown();

    QPushButton *refreshBtn = new QPushButton("🗘", this);
    refreshBtn->setFixedSize(30, 30);
    refreshBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #3498db; "
        "    color: white; "
        "    border-radius: 5px; "
        "    font-size: 18px; "
        "    border: none;"
        "    text-align: center;"
        "    padding: 0px;"
        "    padding-top: 5px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #21618c;"
        "}"
        );
    refreshBtn->setToolTip("Refresh IP Address");

    myIpLayout->addWidget(myIpCombo);
    myIpLayout->addWidget(refreshBtn);
    networkLayout->addLayout(myIpLayout);

    QLabel *myIpHelp = new QLabel("Share this with the person you want to call", this);
    myIpHelp->setStyleSheet("font-size: 11px; color: #7f8c8d; margin-bottom: 15px; background-color: transparent;");
    networkLayout->addWidget(myIpHelp);

    // Peer IP Address
    QLabel *peerIpLabel = new QLabel("Peer IP Address:", this);
    peerIpLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #34495e; background-color: transparent;");
    networkLayout->addWidget(peerIpLabel);

    peerIpEdit = new QLineEdit(this);
    peerIpEdit->setPlaceholderText("192.168.1.100");
    peerIpEdit->setStyleSheet(
        "QLineEdit {"
        "    padding: 5px 12px; "
        "    border: 2px solid #bdc3c7; "
        "    border-radius: 5px; "
        "    font-size: 14px;"
        "    color: #000000;"
        "}"
        "QLineEdit:focus {"
        "    border: 2px solid #3498db;"
        "}"
        );
    networkLayout->addWidget(peerIpEdit);

    QLabel *peerIpHelp = new QLabel("Enter the IP address of the person you're calling", this);
    peerIpHelp->setStyleSheet("font-size: 11px; color: #7f8c8d; margin-bottom: 15px; background-color: transparent;");
    networkLayout->addWidget(peerIpHelp);

    // Username (hidden by default)
    usernameLabel = new QLabel("Your Username:", this);
    usernameLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #34495e; background-color: transparent;");
    usernameLabel->setVisible(false);
    networkLayout->addWidget(usernameLabel);

    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("john_doe");
    usernameEdit->setMaxLength(20);
    usernameEdit->setStyleSheet(
        "QLineEdit {"
        "    padding: 12px; "
        "    border: 2px solid #bdc3c7; "
        "    border-radius: 5px; "
        "    font-size: 14px;"
        "    color: #000000;"
        "}"
        "QLineEdit:focus {"
        "    border: 2px solid #3498db;"
        "}"
        );
    usernameEdit->setVisible(false);
    networkLayout->addWidget(usernameEdit);

    usernameHelp = new QLabel("Used for authentication (3-20 characters, alphanumeric + underscore)", this);
    usernameHelp->setStyleSheet("font-size: 11px; color: #7f8c8d; background-color: transparent;");
    usernameHelp->setVisible(false);
    networkLayout->addWidget(usernameHelp);

    mainLayout->addWidget(networkGroup);

    // ========== STATUS BAR ==========
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(15);

    statusLabel = new QLabel("● Ready to connect", this);
    statusLabel->setStyleSheet(
        "padding: 15px; "
        "background-color: #d4edda; "
        "color: #155724; "
        "border: 1px solid #c3e6cb; "
        "border-radius: 8px; "
        "font-weight: bold; "
        "font-size: 13px;"
        );
    statusLabel->setAlignment(Qt::AlignCenter);
    bottomLayout->addWidget(statusLabel, 2);

    // ========== CONNECT BUTTON ==========
    connectBtn = new QPushButton("Connect", this);
    connectBtn->setFixedHeight(55);
    connectBtn->setCursor(Qt::PointingHandCursor);
    connectBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #27ae60; "
        "    color: white; "
        "    font-size: 16px; "
        "    font-weight: bold; "
        "    border-radius: 27px; "
        "    border: none;"
        "}"
        "QPushButton:hover {"
        "    background-color: #229954;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #1e8449;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #95a5a6;"
        "}"
        );
    bottomLayout->addWidget(connectBtn, 1);

    mainLayout->addStretch();

    mainLayout->addLayout(bottomLayout);

    // mainLayout->addStretch();

    // ========== CONNECT SIGNALS ==========
    connect(clientRadio, &QRadioButton::toggled, this, &LaunchWindow::onModeChanged);
    connect(refreshBtn, &QPushButton::clicked, this, &LaunchWindow::onRefreshIpClicked);
    connect(connectBtn, &QPushButton::clicked, this, &LaunchWindow::onConnectClicked);
}

void LaunchWindow::onModeChanged()
{
    bool isClient = clientRadio->isChecked();
    usernameLabel->setVisible(isClient);
    usernameEdit->setVisible(isClient);
    usernameHelp->setVisible(isClient);
}

void LaunchWindow::onRefreshIpClicked()
{
    populateIpDropdown();
    updateStatusMessage("IP address refreshed", "info");
}

void LaunchWindow::onConnectClicked()
{
    QString peerIp = peerIpEdit->text().trimmed();
    QString username = usernameEdit->text().trimmed();
    bool isServer = serverRadio->isChecked();
    QString selectedIp = myIpCombo->currentText().trimmed();

    // Validation
    if (peerIp.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please enter the peer IP address.");
        return;
    }

    if (!validateIP(peerIp)) {
        QMessageBox::warning(this, "Validation Error", "Invalid IP address format.\nExample: 192.168.1.100");
        return;
    }

    if (!isServer && username.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please enter your username.");
        return;
    }

    if (!isServer) {
        QRegularExpression usernameRegex("^[a-zA-Z0-9_]{3,20}$");
        if (!usernameRegex.match(username).hasMatch()) {
            QMessageBox::warning(this, "Validation Error",
                                 "Username must be 3-20 characters long.\n"
                                 "Only letters, numbers, and underscore allowed.");
            return;
        }
    }

    // Stop any existing backend process
    if (backendProcess->state() != QProcess::NotRunning) {
        backendProcess->kill();
        backendProcess->waitForFinished();
    }

    // Connect backend signals
    connect(backendProcess, &QProcess::started, this, &LaunchWindow::onBackendStarted);
    connect(backendProcess, &QProcess::readyReadStandardOutput, this, &LaunchWindow::onBackendOutput);
    connect(backendProcess, &QProcess::readyReadStandardError, this, &LaunchWindow::onBackendError);
    connect(backendProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &LaunchWindow::onBackendFinished);

    // Build path to backend executable
    QString backendPath = QCoreApplication::applicationDirPath() + "/../../../backend/";
    QString executable;
    QStringList arguments;

    if (isServer) {
        executable = backendPath + "server";
        arguments << peerIp;  // server takes peer IP as argument

        updateStatusMessage("Starting server...", "info");
    } else {
        executable = backendPath + "client";
        arguments << peerIp << username;  // client takes IP and username

        updateStatusMessage("Connecting to server...", "info");
    }

    // Disable connect button while connecting
    connectBtn->setEnabled(false);
    connectBtn->setText("Connecting...");

    // Launch backend
    backendProcess->start(executable, arguments);

    if (!backendProcess->waitForStarted(3000)) {
        updateStatusMessage("Failed to start backend", "error");
        QMessageBox::critical(this, "Backend Error",
                              "Could not start backend process.\n\n"
                              "Executable: " + executable + "\n"
                                                 "Make sure backend is compiled:\n"
                                                 "cd backend && make");
        connectBtn->setEnabled(true);
        connectBtn->setText("Connect");
    }
}

void LaunchWindow::updateStatusMessage(const QString &message, const QString &type)
{
    statusLabel->setText("● " + message);

    if (type == "success") {
        statusLabel->setStyleSheet(
            "padding: 15px; "
            "background-color: #d4edda; "
            "color: #155724; "
            "border: 1px solid #c3e6cb; "
            "border-radius: 8px; "
            "font-weight: bold; "
            "font-size: 13px;"
            );
    } else if (type == "error") {
        statusLabel->setStyleSheet(
            "padding: 15px; "
            "background-color: #f8d7da; "
            "color: #721c24; "
            "border: 1px solid #f5c6cb; "
            "border-radius: 8px; "
            "font-weight: bold; "
            "font-size: 13px;"
            );
    } else if (type == "info") {
        statusLabel->setStyleSheet(
            "padding: 15px; "
            "background-color: #d1ecf1; "
            "color: #0c5460; "
            "border: 1px solid #bee5eb; "
            "border-radius: 8px; "
            "font-weight: bold; "
            "font-size: 13px;"
            );
    }
}

QStringList LaunchWindow::getAllLocalIPs()
{
    QStringList ipList;
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    for (const QNetworkInterface &interface : interfaces) {
        // Skip loopback and down interfaces
        if (interface.flags().testFlag(QNetworkInterface::IsUp) &&
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {

            QList<QNetworkAddressEntry> entries = interface.addressEntries();
            for (const QNetworkAddressEntry &entry : entries) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    // Add only the IP address (no interface name)
                    ipList.append(entry.ip().toString());
                }
            }
        }
    }

    if (ipList.isEmpty()) {
        ipList.append("127.0.0.1");
    }

    return ipList;
}

// UPDATED: Populate the IP dropdown
void LaunchWindow::populateIpDropdown()
{
    myIpCombo->clear();

    // Get all interfaces with their names for selection logic
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    QStringList ipList;
    int wirelessIndex = -1;
    int currentIndex = 0;

    for (const QNetworkInterface &interface : interfaces) {
        if (interface.flags().testFlag(QNetworkInterface::IsUp) &&
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {

            QList<QNetworkAddressEntry> entries = interface.addressEntries();
            for (const QNetworkAddressEntry &entry : entries) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    QString ip = entry.ip().toString();
                    ipList.append(ip);

                    // Check if this is a wireless interface
                    QString interfaceName = interface.humanReadableName().toLower();
                    if ((interfaceName.contains("wlp") || interfaceName.contains("wlan") ||
                         interfaceName.contains("wi-fi") || interfaceName.contains("wireless"))
                        && wirelessIndex == -1) {
                        wirelessIndex = currentIndex;
                    }
                    currentIndex++;
                }
            }
        }
    }

    if (ipList.isEmpty()) {
        ipList.append("127.0.0.1");
    }

    myIpCombo->addItems(ipList);

    // Set wireless interface as default if found
    if (wirelessIndex != -1) {
        myIpCombo->setCurrentIndex(wirelessIndex);
    }
}

bool LaunchWindow::validateIP(const QString &ip)
{
    QRegularExpression ipRegex(
        "^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
        "(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"
        );

    return ipRegex.match(ip).hasMatch();
}

void LaunchWindow::onBackendStarted()
{
    qDebug() << "Backend process started successfully";
    updateStatusMessage("Backend started, authenticating...", "info");
}

void LaunchWindow::onBackendOutput()
{
    QString output = QString::fromUtf8(backendProcess->readAllStandardOutput());
    qDebug() << "Backend output:" << output;

    // Parse backend output to update status
    if (output.contains("Key exchange successful", Qt::CaseInsensitive)) {
        updateStatusMessage("Key exchange successful", "success");
    }
    else if (output.contains("Authentication successful", Qt::CaseInsensitive)) {
        updateStatusMessage("Authentication successful", "success");
    }
    else if (output.contains("Connected", Qt::CaseInsensitive)) {
        updateStatusMessage("Connected - Call started!", "success");
        // Could open a call window here in the future
    }
    else if (output.contains("Waiting", Qt::CaseInsensitive)) {
        updateStatusMessage("Waiting for peer...", "info");
    }
}

void LaunchWindow::onBackendError()
{
    QString error = QString::fromUtf8(backendProcess->readAllStandardError());
    qDebug() << "Backend error:" << error;

    // Show errors in status bar
    if (!error.trimmed().isEmpty()) {
        updateStatusMessage("Error: " + error.left(50), "error");
    }
}

void LaunchWindow::onBackendFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Backend finished. Exit code:" << exitCode << "Status:" << exitStatus;

    // Re-enable connect button
    connectBtn->setEnabled(true);
    connectBtn->setText("Connect");

    if (exitStatus == QProcess::CrashExit) {
        updateStatusMessage("Backend crashed", "error");
        QMessageBox::warning(this, "Backend Crashed",
                             "The backend process crashed unexpectedly.");
    }
    else if (exitCode != 0) {
        updateStatusMessage("Backend exited with error", "error");
    }
    else {
        updateStatusMessage("Call ended", "info");
    }
}
