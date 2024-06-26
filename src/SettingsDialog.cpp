
#include "SettingsDialog.h"

SettingsDialog::SettingsDialog(QSettings& settings, const QIcon& icon, QWidget* parent) : QDialog(parent), settings(settings) {
    setupUi(icon);
    loadSettings();
}

void SettingsDialog::setupUi(const QIcon& icon) {
    setWindowIcon(icon);
    setMinimumWidth(300);

    auto mainLayout = new QVBoxLayout(this);
    auto urlLayout = new QHBoxLayout();
    auto iconLayout = new QHBoxLayout();

    auto title = new QLabel("Settings", this);
    title->setMaximumHeight(30);
    title->setAlignment(Qt::AlignCenter);
    QFont font;
    font.setBold(true);
    title->setFont(font);

    auto urlLabel = new QLabel("URL:", this);
    urlText = new QLineEdit(this);
    urlLayout->addWidget(urlLabel);
    urlLayout->addWidget(urlText);

    auto iconLabel = new QLabel("Icon:", this);
    iconBox = new QComboBox(this);
    iconBox->addItem(C_ICON_COLOR);
    iconBox->addItem(C_ICON_WHITE);
    iconBox->addItem(C_ICON_BLACK);
    iconBox->addItem(C_ICON_SYSTEM);
    iconLayout->addWidget(iconLabel);
    iconLayout->addWidget(iconBox);

    autostartBox = new QCheckBox("autostart on login", this);
    notificationBox = new QCheckBox("disable start/stop notifications", this);
    notificationBox->setToolTip("affects start and stop service notifications only - errors will still be shown");

    createBGService = new QPushButton("install as system service", this);

    auto confirmButtons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);

    mainLayout->addWidget(title);
    mainLayout->addLayout(urlLayout);
    mainLayout->addLayout(iconLayout);
    mainLayout->addWidget(autostartBox);
    mainLayout->addWidget(notificationBox);
    mainLayout->addWidget(createBGService);
    mainLayout->addWidget(confirmButtons);

    connect(createBGService, &QPushButton::clicked, this, &SettingsDialog::showServiceDialog);
    connect(confirmButtons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(confirmButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void SettingsDialog::loadSettings() {
    settings.sync();
    urlText->setText(settings.value(C_URL).toString());
    iconBox->setCurrentText(settings.value(C_ICON).toString());
    autostartBox->setChecked(settings.value(C_AUTOSTART).toBool());
    notificationBox->setChecked(not settings.value(C_NOTIFICATION).toBool());
}

void SettingsDialog::saveSettings() {
    qDebug() << "saving settings";

    settings.setValue(C_URL, urlText->text());
    settings.setValue(C_ICON, iconBox->currentText());
    settings.setValue(C_AUTOSTART, autostartBox->isChecked());
    settings.setValue(C_NOTIFICATION, not notificationBox->isChecked());
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
