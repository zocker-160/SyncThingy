
#include "SettingsDialog.h"

SettingsDialog::SettingsDialog(QSettings& settings, const QIcon& icon, QWidget* parent) : QDialog(parent), settings(settings) {
    setupUi(icon);
    loadSettings();
}

void SettingsDialog::setupUi(const QIcon& icon) {
    setWindowIcon(icon);
    setMinimumWidth(300);

    // Title
    auto title = new QLabel("Settings", this);
    title->setMaximumHeight(30);
    title->setAlignment(Qt::AlignCenter);
    QFont font;
    font.setBold(true);
    title->setFont(font);

    // IP - Port
    auto ipBox = new QGroupBox("IP", this);
    auto ipBoxLayout = new QHBoxLayout(ipBox);

    ipText = new QLineEdit(this);
    ipText->setPlaceholderText("127.0.0.1");
    ipBoxLayout->addWidget(ipText);
    //
    auto portBox = new QGroupBox("Port", this);
    auto portBoxLayout = new QHBoxLayout(portBox);

    portInput = new QSpinBox(this);
    portInput->setMinimum(1024);
    portInput->setMaximum(65535);
    portBoxLayout->addWidget(portInput);
    //
    auto urlBoxLayout = new QHBoxLayout();
    urlBoxLayout->addWidget(ipBox);
    urlBoxLayout->addWidget(portBox);

    // Icon selector
    auto iconBox = new QGroupBox("Icon", this);
    auto iconBoxLayout = new QHBoxLayout(iconBox);

    iconSelector = new QComboBox(this);
    iconSelector->addItem(C_ICON_COLOR);
    iconSelector->addItem(C_ICON_WHITE);
    iconSelector->addItem(C_ICON_BLACK);
    iconSelector->addItem(C_ICON_SYSTEM);
    iconBoxLayout->addWidget(iconSelector);

    // Misc
    auto miscBox = new QGroupBox("Misc", this);
    auto miscBoxLayout = new QVBoxLayout(miscBox);

    autostartBox = new QCheckBox("autostart on login", this);
    notificationBox = new QCheckBox("show start/stop notifications", this);
    notificationBox->setToolTip("affects start / stop service notifications only - errors will still be shown");
    miscBoxLayout->addWidget(autostartBox);
    miscBoxLayout->addWidget(notificationBox);

    // Buttons
    createBGService = new QPushButton("install as system service", this);
    auto confirmButtons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);

    // Main layout
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(title);
    mainLayout->addLayout(urlBoxLayout);
    mainLayout->addWidget(iconBox);
    mainLayout->addWidget(miscBox);
    mainLayout->addWidget(createBGService);
    mainLayout->addWidget(confirmButtons);

    connect(createBGService, &QPushButton::clicked, this, &SettingsDialog::showServiceDialog);
    connect(confirmButtons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(confirmButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void SettingsDialog::loadSettings() {
    qDebug() << "loading settings";

    settings.sync();

    ipText->setText(settings.value(C_IP).toString());
    portInput->setValue(settings.value(C_PORT).toInt());
    iconSelector->setCurrentText(settings.value(C_ICON).toString());

    autostartBox->setChecked(settings.value(C_AUTOSTART).toBool());
    notificationBox->setChecked(settings.value(C_NOTIFICATION).toBool());
}

void SettingsDialog::saveSettings() {
    qDebug() << "saving settings";

    settings.setValue(C_IP, ipText->text());
    settings.setValue(C_PORT, portInput->value());
    settings.setValue(C_ICON, iconSelector->currentText());
    settings.setValue(C_AUTOSTART, autostartBox->isChecked());
    settings.setValue(C_NOTIFICATION, notificationBox->isChecked());

    settings.sync();
}

void SettingsDialog::accept() {
    saveSettings();
    QDialog::accept();
}

void SettingsDialog::showServiceDialog() {
    ServiceDialog serviceDialog(this);
    serviceDialog.exec();
}
