
#include "ServiceDialog.h"

const char* installTitle = "Run in terminal to install SyncThingy as system service";
const char* uninstallTitle = "Run in terminal to uninstall SyncThingy as system service";

const char* systemdInstallCommand =
        "mkdir -p ~/.config/systemd/user \n"
        "wget https://raw.githubusercontent.com/zocker-160/SyncThingy/master/linux_packaging/syncthingy.service -O ~/.config/systemd/user/syncthingy.service \n"
        "systemctl --user enable --now syncthingy";

const char* systemdUninstallCommand =
        "systemctl --user stop syncthingy \n"
        "systemctl --user disable syncthingy";

ServiceDialog::ServiceDialog(QWidget *parent) : QDialog(parent) {
    setupUi();
    resize(650, 320);
}

void ServiceDialog::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    auto lowerRow = new QHBoxLayout();

    title = new QLabel(installTitle, this);
    title->setMaximumHeight(30);
    title->setAlignment(Qt::AlignCenter);

    installCommandTextbox = new QTextBrowser(this);
    installCommandTextbox->setText(systemdInstallCommand);
    installCommandTextbox->setStyleSheet("color: white; background-color: black;");

    copyToClipboardBtn = new QPushButton(QIcon::fromTheme("document-save"), "copy to clipboard", this);
    helpINeedMommy = new QPushButton(QIcon::fromTheme("help-browser"), "HELP, I have no idea what to do!", this);

    showUninstall = new QCheckBox("show uninstall command", this);

    auto confirmButtons = new QDialogButtonBox(QDialogButtonBox::Close);

    lowerRow->addWidget(helpINeedMommy);
    lowerRow->addWidget(confirmButtons);

    mainLayout->addWidget(title);
    mainLayout->addWidget(installCommandTextbox);
    mainLayout->addWidget(copyToClipboardBtn);
    mainLayout->addWidget(showUninstall);
    mainLayout->addLayout(lowerRow);

    connect(copyToClipboardBtn, &QPushButton::clicked, this, &ServiceDialog::copyToClipboard);
    connect(showUninstall, &QCheckBox::stateChanged, this, &ServiceDialog::toggleCommandContent);
    connect(helpINeedMommy, &QPushButton::clicked, this, &ServiceDialog::openHelpPage);
    connect(confirmButtons, &QDialogButtonBox::helpRequested, this, &ServiceDialog::openHelpPage);
    connect(confirmButtons, &QDialogButtonBox::clicked, this, &QDialog::accept);
}

void ServiceDialog::copyToClipboard() {
    auto* clipboard = SingleApplication::clipboard();
    if (showUninstall->isChecked())
        clipboard->setText(systemdUninstallCommand);
    else
        clipboard->setText(systemdInstallCommand);

    qDebug() << "copied to clipboard";
}

void ServiceDialog::toggleCommandContent() {
    qDebug() << "toggle install / uninstall command";

    installCommandTextbox->clear();

    if (showUninstall->isChecked()) {
        title->setText(uninstallTitle);
        installCommandTextbox->setText(systemdUninstallCommand);
    } else {
        title->setText(installTitle);
        installCommandTextbox->setText(systemdInstallCommand);
    }
}

void ServiceDialog::openHelpPage() {
    qDebug() << "opening Syncthing help page using xdg-open";
    system(QString("xdg-open ").append(C_GITHUB_HELP).toStdString().c_str());
}
