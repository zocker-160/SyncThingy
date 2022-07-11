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

#define VERSION "v0.3"
#define APP_NAME "SyncThingy"

class TrayIcon: public QSystemTrayIcon {

public:
    explicit TrayIcon() {
        settings = new QSettings("SyncThingy", "settings");

        initSettings();
        setupUi();
        setupProcess();
        setupTimer();
    }

    bool syncthingRunning() {
        return syncthingProcess != nullptr && syncthingProcess->state() == QProcess::Running;
    }

public slots:
    void stopProcess() {
        std::cout << "quit triggered \n";

        if (syncthingRunning()) {
            syncthingProcess->terminate();
            syncthingProcess->waitForFinished();
        }

        QString msg = QString("exit code: ").append(QString::number(syncthingProcess->exitCode()));

        if (syncthingProcess->exitCode() == 0)
            showMessage("Syncthing stopped", msg, icon(), 5000);
    };

private:
    QSettings* settings;
    QTimer* timer;
    QProcess* syncthingProcess;

    void setupUi() {
        const char* flatpakID = std::getenv("FLATPAK_ID");
        QString prefix;

        if (flatpakID == nullptr) {
            prefix = "syncthing";
        } else {
            qDebug() << "running inside Flatpak \n";
            prefix = flatpakID;
        }

        auto iconType = settings->value("icon").toString();
        if (iconType == "white" or iconType == "black")
            iconType = prefix + "." + iconType;
        else
            iconType = prefix;

        qDebug() << "Using Icon:" << iconType;
        setIcon(QIcon::fromTheme(iconType));

        auto openGitHubAction = new QAction(QString(APP_NAME).append(" ").append(VERSION), this);
        auto showBrowserAction = new QAction("Open WebUI", this);
        auto openConfigAction = new QAction("Open Config", this);
        auto exitAction = new QAction("Exit", this);
        auto menu = new QMenu();

        openGitHubAction->setIcon(QIcon::fromTheme("help-about"));
        showBrowserAction->setIcon(QIcon::fromTheme("help-browser"));
        openConfigAction->setIcon(QIcon::fromTheme("text"));
        exitAction->setIcon(QIcon::fromTheme("application-exit"));

        connect(openGitHubAction, &QAction::triggered, this, &TrayIcon::showGitHub);
        connect(showBrowserAction, &QAction::triggered, this, &TrayIcon::showBrowser);
        connect(openConfigAction, &QAction::triggered, this, &TrayIcon::openConfig);
        connect(exitAction, &QAction::triggered, QApplication::instance(), &QApplication::quit);
        connect(this, &TrayIcon::activated, this, &TrayIcon::handleActivation);

        menu->addAction(openGitHubAction);
        menu->addSeparator();
        menu->addSeparator();
        menu->addAction(showBrowserAction);
        menu->addAction(openConfigAction);
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

        showMessage("Syncthing started", "", icon(), 3000);
    }

    void setupTimer() {
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &TrayIcon::checkSyncthingRunning);
        timer->start(5000);
    }

    void initSettings() {
        if (not settings->contains("icon")) {
            settings->setValue("url", "http://127.0.0.1:8384");
            settings->setValue("icon", "default");
            settings->sync();
        }
    }

    static bool checkSyncthingAvailable() {
        int ret = system("which syncthing");
        return ret == 0;
    }

private slots:
    void showBrowser() {
        system(QString("xdg-open ").append(settings->value("url").toString()).toStdString().c_str());
    };

    void openConfig() {
        system(QString("xdg-open ").append(settings->fileName()).toStdString().c_str());
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

    void checkSyncthingRunning() {
        if (not syncthingRunning())
            QApplication::quit();
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName(APP_NAME);
    QApplication::setApplicationVersion(VERSION);

    TrayIcon tray;

    QObject::connect(&app, &QApplication::aboutToQuit, &tray, &TrayIcon::stopProcess);

    if (not tray.syncthingRunning())
        return 1;

    return QApplication::exec();
}
