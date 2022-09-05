#include <QApplication>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>
#include <QProcess>
#include <QStringList>
#include <QTimer>
#include <QSettings>

#include <QDebug>

#include <cstdlib>
#include <iostream>

#include <libportal-qt5/portal-qt5.h>

#include "SettingsDialog.h"

#define VERSION "v0.5"
#define APP_NAME "SyncThingy"

class TrayIcon: public QSystemTrayIcon {

public:
    explicit TrayIcon(QSettings& settings) : QSystemTrayIcon(), settings(settings) {
        initSettings();
        setupUi();
        setupProcess();
        setupTimer();
        requestBackgroundPermission();
    }

    bool syncthingRunning() {
        return syncthingProcess != nullptr && syncthingProcess->state() == QProcess::Running;
    }

//public slots:
    void stopProcess() {
        std::cout << "quit triggered \n";

        if (syncthingRunning()) {
            syncthingProcess->terminate();
            syncthingProcess->waitForFinished();
        }

        QString msg = QString("exit code: ").append(QString::number(syncthingProcess->exitCode()));

        if (syncthingProcess->exitCode() == 0)
            _showMessage("Syncthing stopped", msg, icon(), 5000);
    };

private:
    QSettings& settings;
    QTimer* timer;
    QProcess* syncthingProcess;

    void setupUi() {
        updateIcon();

        auto openGitHubAction = new QAction(QString(APP_NAME).append(" ").append(VERSION), this);
        auto showBrowserAction = new QAction("Open WebUI", this);
        auto openConfigAction = new QAction("Open Config", this);
        auto openSettingsAction = new QAction("Settings...", this);
        auto exitAction = new QAction("Exit", this);
        auto menu = new QMenu();

        openGitHubAction->setIcon(QIcon::fromTheme("help-about"));
        showBrowserAction->setIcon(QIcon::fromTheme("help-browser"));
        openConfigAction->setIcon(QIcon::fromTheme("text"));
        openSettingsAction->setIcon(QIcon::fromTheme("preferences-desktop-personal"));
        exitAction->setIcon(QIcon::fromTheme("application-exit"));

        connect(openGitHubAction, &QAction::triggered, this, &TrayIcon::showGitHub);
        connect(showBrowserAction, &QAction::triggered, this, &TrayIcon::showBrowser);
        connect(openSettingsAction, &QAction::triggered, this, &TrayIcon::showSettingsDialog);
        connect(openConfigAction, &QAction::triggered, this, &TrayIcon::openConfig);
        connect(exitAction, &QAction::triggered, QApplication::instance(), &QApplication::quit);
        connect(this, &TrayIcon::activated, this, &TrayIcon::handleActivation);

        menu->addAction(openGitHubAction);
        menu->addSeparator();
        menu->addSeparator();
        menu->addAction(showBrowserAction);
        menu->addAction(openSettingsAction);

        //menu->addSeparator();
        //menu->addAction(openConfigAction);

        menu->addSeparator();
        menu->addAction(exitAction);

        setContextMenu(menu);
        show();
    }

    void setupProcess() {
        if (not checkSyncthingAvailable()) {
            const char* msg = "Syncthing could not be found in PATH!";

            std::cout << "ERROR: "<< msg << std::endl;
            showMessage("ERROR", msg, QSystemTrayIcon::Critical, 0);

            return;
        }

        QStringList arguments;
        arguments << "serve" << "--no-browser" << "--logfile=default";

        syncthingProcess = new QProcess(this);
        syncthingProcess->start("syncthing", arguments);
        syncthingProcess->waitForStarted();

        _showMessage("Syncthing started", "", icon(), 3000);
    }

    void setupTimer() {
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &TrayIcon::checkSyncthingRunning);
        timer->start(5000);
    }

    void updateIcon() {
        const char* flatpakID = std::getenv("FLATPAK_ID");
        QString prefix;

        if (flatpakID == nullptr) {
            prefix = "syncthing";
        } else {
            qDebug() << "running inside Flatpak \n";
            prefix = flatpakID;
        }

        QString iconType = settings.value("icon").toString();
        if (iconType == "white" or iconType == "black")
            iconType = prefix + "." + iconType;
        else
            iconType = prefix;

        qDebug() << "Using Icon:" << iconType;
        setIcon(QIcon::fromTheme(iconType));
    }

    void initSettings() {
        if (not settings.contains(C_URL)) {
            settings.setValue(C_URL, "http://127.0.0.1:8384");
            settings.setValue(C_ICON, "default");
            settings.sync();
        }
        // new setting in 0.4 which needs to be true by default
        if (not settings.contains(C_AUTOSTART)) {
            settings.setValue(C_AUTOSTART, true);
            settings.sync();
        }
        // new setting in 0.5 which needs to be true by default
        if (not settings.contains(C_NOTIFICATION)) {
            settings.setValue(C_NOTIFICATION, true);
            settings.sync();
        }
    }

    void requestBackgroundPermission() {
        qDebug() << "Requesting background permission...";

        auto commandline = g_ptr_array_new();
        g_ptr_array_add(commandline, (gpointer) "SyncThingy");

        char reason[] = "Reason: Ability to sync data in the background.";
        auto flag = settings.value(C_AUTOSTART).toBool() ? XDP_BACKGROUND_FLAG_AUTOSTART : XDP_BACKGROUND_FLAG_NONE;

        xdp_portal_request_background(
            XdpQt::globalPortalObject(),
            nullptr,
            reason,
            commandline,
            flag,
            nullptr,
            TrayIcon::backgroundRequestCallback,
            this
        );
    }

    static void backgroundRequestCallback(GObject* object, GAsyncResult* result, void* data) {
        GError* error = nullptr;
        auto ret = xdp_portal_request_background_finish(
                XdpQt::globalPortalObject(), result, &error);

        auto tray = static_cast<TrayIcon*>(data);

        if (ret)
            qDebug() << "Background / Autostart permission granted";
        else {
            qDebug() << "Background / Autostart permission revoked";
            tray->showMessage(
                "Background permission revoked",
                "SyncThingy might not work as expected!",
                tray->icon(),
                5000
            );
        }
    }

    static bool checkSyncthingAvailable() {
        int ret = system("which syncthing");
        return ret == 0;
    }

//private slots:
    void showBrowser() {
        system(QString("xdg-open ").append(settings.value(C_URL).toString()).toStdString().c_str());
    };

    void openConfig() {
        system(QString("xdg-open ").append(settings.fileName()).toStdString().c_str());
    }

    static void showGitHub() {
        system("xdg-open https://github.com/zocker-160/SyncThingy");
    }

    void handleActivation(QSystemTrayIcon::ActivationReason reason) {
        switch (reason) {
            case QSystemTrayIcon::Trigger:
                showBrowser();
        }
    }

    void showSettingsDialog() {
        qDebug() << "open settings";

        SettingsDialog options(settings, icon());
        if (options.exec() == QDialog::Accepted) {
            updateIcon();
            requestBackgroundPermission();
        }
    }

    void checkSyncthingRunning() {
        if (not syncthingRunning())
            QApplication::quit();
    }

    void _showMessage(const QString& title, const QString& msg, const QIcon& icon, int msecs = 10000) {
        if (settings.value(C_NOTIFICATION).toBool())
            showMessage(title, msg, icon, msecs);
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName(APP_NAME);
    QApplication::setApplicationVersion(VERSION);

    QSettings settings(APP_NAME, "settings");
    TrayIcon tray(settings);

    QObject::connect(&app, &QApplication::aboutToQuit, &tray, &TrayIcon::stopProcess);

    if (not tray.syncthingRunning())
        return 1;

    return QApplication::exec();
}
