#include <QApplication>
#include <QDialog>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>
#include <QProcess>
#include <QStringList>
#include <QTimer>
#include <QSettings>
#include <QSemaphore>
#include <QFileSystemWatcher>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>

#include <cstdlib>
#include <iostream>

#include <libportal-qt5/portal-qt5.h>

#include <SingleApplication>
#include "SettingsDialog.h"

#define VERSION "v0.7.2"
#define APP_NAME "SyncThingy"

class SyncThingy : public QDialog {

public:
    explicit SyncThingy(QSettings& settings) : settings(settings) {
        initSettings();
        initSettingsWatcher(true);
        workaroundFuckingStupidGTKbug();
        setupUi();
        checkRunning();
        requestBackgroundPermission();
    }

    ~SyncThingy() override {
        delete semaphore;
    }

//public slots:
    void stopProcess() {
        qDebug() << "quit triggered \n";

        if (syncthingProcessRunning()) {
            syncthingProcess->terminate();
            syncthingProcess->waitForFinished();
        }

        int exitcode = syncthingProcess->exitCode();

        if (exitcode == 0) {
            const auto msg = QString("exit code (0)");
            _showMessage("Syncthing stopped", msg, trayIcon->icon(), 5000);
        } else {
            const auto msg = QString("Syncthing failed to start! exit code (")
                    .append(QString::number(exitcode))
                    .append(")");
            trayIcon->showMessage("ERROR", msg, QSystemTrayIcon::Critical, 0);
        }
    };

    static void secondaryStarted() {
        qDebug() << "Secondary SyncThingy instance started!!";
        //_showMessage("INFO", "SEC STARTED", trayIcon->icon(), 0);
        //trayIcon->show();
    }

private:
    QSettings& settings;
    QFileSystemWatcher* settingsWatcher = new QFileSystemWatcher(this);
    QTimer* timer = new QTimer(this);
    QProcess* syncthingProcess = new QProcess(this);

    QSemaphore* semaphore = new QSemaphore(1);
    QSystemTrayIcon* trayIcon = new QSystemTrayIcon(this);

    QNetworkAccessManager networkManager;

    void workaroundFuckingStupidGTKbug() {
        // Yes I know this ridiculously stupid, but I have to work around a stupid GTK / GNOME bug
        // see https://github.com/zocker-160/SyncThingy/issues/8
        resize(0, 0);
    }

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

        connect(openGitHubAction, &QAction::triggered, this, &SyncThingy::showGitHub);
        connect(showBrowserAction, &QAction::triggered, this, &SyncThingy::showBrowser);
        connect(openSettingsAction, &QAction::triggered, this, &SyncThingy::showSettingsDialog);
        connect(openConfigAction, &QAction::triggered, this, &SyncThingy::openConfig);
        connect(exitAction, &QAction::triggered, QApplication::instance(), &QApplication::quit);
        connect(trayIcon, &QSystemTrayIcon::activated, this, &SyncThingy::handleActivation);

        menu->addAction(openGitHubAction);
        menu->addSeparator();
        menu->addSeparator();
        menu->addAction(showBrowserAction);
        menu->addAction(openSettingsAction);

        //menu->addSeparator();
        //menu->addAction(openConfigAction);

        menu->addSeparator();
        menu->addAction(exitAction);

        trayIcon->setContextMenu(menu);
        trayIcon->show();
    }

    void checkRunning() {
        auto reply = _requestSyncthingHealth(true);
        connect(reply, &QNetworkReply::finished, [=] {
            if (_isSyncthingRunning(reply, true)) {
                qDebug() << "Syncthing instance seems to be already running, Tray Icon only";
            } else {
                qDebug() << "Syncthing does not seem to be running, starting own instance";
                setupProcess();
            }
            setupTimer();
            reply->deleteLater();
        });
        connect(reply, &QNetworkReply::errorOccurred, [=](QNetworkReply::NetworkError error) {
            qDebug() << "ERROR: " << error;

            if (error == QNetworkReply::SslHandshakeFailedError) {
                trayIcon->showMessage("ERROR", "SSL Handshake failed", QSystemTrayIcon::Critical, 0);
                QApplication::quit();
            }
        });
    }

    QNetworkReply* _requestSyncthingHealth(bool debugPrint) {
        const auto pingEndpoint = QString(settings.value(C_URL).toString()).append("/rest/noauth/health");
        if (debugPrint)
            qDebug() << "Ping endpoint: " << pingEndpoint;

        QNetworkRequest request(pingEndpoint);
        QSslConfiguration sslConf = request.sslConfiguration();
        sslConf.setPeerVerifyMode(QSslSocket::VerifyNone);
        request.setSslConfiguration(sslConf);
        request.setRawHeader("Content-Type", "application/json; charset=utf-8");
        request.setRawHeader("Accept", "application/json");

        return networkManager.get(request);
    }

    static bool _isSyncthingRunning(QNetworkReply* reply, bool debugPrint) {
        auto jsonDoc = QJsonDocument::fromJson(reply->readAll());
        auto status = jsonDoc.object().value("status");

        if (debugPrint)
            qDebug() << "reply: " << jsonDoc;

        if (not status.isUndefined() && status.toString() == "OK") {
            return true;
        } else {
            qDebug() << "syncthing is not running";
            return false;
        }
    }

    void setupProcess() {
        if (not checkSyncthingAvailable()) {
            const char* msg = "Syncthing could not be found in PATH!";

            std::cout << "ERROR: "<< msg << std::endl;
            trayIcon->showMessage("ERROR", msg, QSystemTrayIcon::Critical, 0);

            return;
        }

        QStringList arguments;
        arguments << "serve" << "--no-browser" << "--logfile=default";

        syncthingProcess->start("syncthing", arguments);
        syncthingProcess->waitForStarted();

        _showMessage("SyncThingy started", "", trayIcon->icon(), 3000);
    }

    void setupTimer() {
        connect(timer, &QTimer::timeout, this, &SyncThingy::checkSyncthingRunning);
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

        QString iconType = settings.value(C_ICON).toString();
        if (iconType == C_ICON_SYSTEM) {
            auto preferredTheme = requestSystemTheme();
            switch (preferredTheme) {
                case 1:
                    iconType = C_ICON_WHITE;
                    break;
                case 2:
                    iconType = C_ICON_BLACK;
                    break;
                default:
                    iconType = C_ICON_COLOR;
                    break;
            }
        }

        if (iconType == C_ICON_WHITE or iconType == C_ICON_BLACK)
            iconType = prefix + "." + iconType;
        else
            iconType = prefix;

        qDebug() << "Using Icon:" << iconType;
        trayIcon->setIcon(QIcon::fromTheme(iconType));
    }

    void initSettings() {
        if (not settings.contains(C_ICON)) {
            settings.setValue(C_ICON, C_ICON_COLOR);
        }

        if (not settings.contains(C_URL)) {
            settings.setValue(C_URL, "http://127.0.0.1:8384");
        }
        // new setting in 0.4 which needs to be true by default
        if (not settings.contains(C_AUTOSTART)) {
            settings.setValue(C_AUTOSTART, true);
        }
        // new setting in 0.5 which needs to be true by default
        if (not settings.contains(C_NOTIFICATION)) {
            settings.setValue(C_NOTIFICATION, true);
        }

        settings.sync();
    }

    void initSettingsWatcher(bool init) {
        // we need to do this, because QT does recreate the config file when saving
        settingsWatcher->removePath(settings.fileName());

        if (not settingsWatcher->addPath(settings.fileName())) {
            qDebug() << "failed to add" << settings.fileName() << "to watchlist!";
            return;
        }

        if (init) {
            connect(settingsWatcher, &QFileSystemWatcher::fileChanged, [=] {
                qDebug() << "settings changed";
                settings.sync();
                updateIcon();
            });

            qDebug() << "config watcher running";
        }
    }

    static unsigned int requestSystemTheme() {
        qDebug() << "requesting system theme";
        auto connection = QDBusConnection::sessionBus();

        // DBus is such fucking garbage!
        auto message = QDBusMessage::createMethodCall(
                "org.freedesktop.portal.Desktop", // Service name
                "/org/freedesktop/portal/desktop", // Object path
                "org.freedesktop.portal.Settings", // Interface
                "Read" // Method name
        );
        message << "org.freedesktop.appearance" << "color-scheme";

        QDBusReply<QDBusVariant> reply = connection.call(message);
        if (reply.isValid()) {
            auto response = reply.value();
            auto variantValue = response.variant().value<QDBusVariant>().variant(); // Linux desktop portals just fucking suck balls!!

            /*
             * 0: No preference
             * 1: Prefer dark appearance
             * 2: Prefer light appearance
             */
            auto actualValue = variantValue.toUInt();
            qDebug() << "got answer:" << actualValue;
            return actualValue;
        } else {
            qDebug() << "DBus call failed: " << reply.error().message();
            return 0;
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
            SyncThingy::backgroundRequestCallback,
            this
        );
    }

    static void backgroundRequestCallback(GObject* object, GAsyncResult* result, void* data) {
        GError* error = nullptr;
        auto ret = xdp_portal_request_background_finish(
                XdpQt::globalPortalObject(), result, &error);

        auto tray = static_cast<SyncThingy*>(data);

        if (ret)
            qDebug() << "Background / Autostart permission granted";
        else {
            qDebug() << "Background / Autostart permission revoked";
            tray->trayIcon->showMessage(
                "Background permission revoked",
                "SyncThingy might not work as expected!",
                tray->trayIcon->icon(),
                0
            );
        }
    }

    void checkSyncthingRunning() {
        //qDebug() << "run check";

        if (not syncthingProcessRunning()) {
            auto reply = _requestSyncthingHealth(false);
            connect(reply, &QNetworkReply::finished, [=] {
                if (not _isSyncthingRunning(reply, false))
                    QApplication::quit();
                else
                    reply->deleteLater();
            });
        }
    }

    bool syncthingProcessRunning() {
        return syncthingProcess->state() == QProcess::Running;
    }

    static bool checkSyncthingAvailable() {
        int ret = system("which syncthing");
        return ret == 0;
    }

//private slots:
    void showBrowser() {
        qDebug() << "opening Syncthing webui using xdg-open";
        system(QString("xdg-open ").append(settings.value(C_URL).toString()).toStdString().c_str());
    };

    void openConfig() {
        qDebug() << "opening config file using xdg-open";
        system(QString("xdg-open ").append(settings.fileName()).toStdString().c_str());
    }

    static void showGitHub() {
        qDebug() << "opening GitHub page using xdg-open";
        system(QString("xdg-open ").append(C_GITHUB).toStdString().c_str());
    }

    void handleActivation(QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            showBrowser();
        }
    }

    void showSettingsDialog() {
        if (semaphore->tryAcquire()) {
            show(); // see workaroundFuckingStupidGTKbug()

            qDebug() << "open settings";

            SettingsDialog options(settings, trayIcon->icon(), this);
            if (options.exec() == QDialog::Accepted) {
                updateIcon();
                requestBackgroundPermission();

                // reset QFileSystemWatcher because QT does actually delete the file and create a new one
                initSettingsWatcher(false);
            }

            hide(); // see workaroundFuckingStupidGTKbug()
            semaphore->release();
        } else {
            qDebug() << "settings dialog is already open";
        }
    }

    void _showMessage(const QString& title, const QString& msg, const QIcon& icon, int msecs = 10000) {
        if (settings.value(C_NOTIFICATION).toBool())
            trayIcon->showMessage(title, msg, icon, msecs);
    }
};

int main(int argc, char *argv[]) {
    SingleApplication app(argc, argv, true);
    SingleApplication::setApplicationName(APP_NAME);
    SingleApplication::setApplicationVersion(VERSION);

    if (app.isSecondary()) {
        qDebug() << "this instance is secondary, exiting with 4";
        return 4;
    }

    QSettings settings(APP_NAME, "settings");
    SyncThingy sth(settings);

    QObject::connect(&app, &SingleApplication::aboutToQuit, &sth, &SyncThingy::stopProcess);
    QObject::connect(&app, &SingleApplication::instanceStarted, &sth, &SyncThingy::secondaryStarted);

    return SingleApplication::exec();
}
