#include <QApplication>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>
#include <QProcess>
#include <QStringList>
#include <QTimer>

#include <cstdlib>
#include <sstream>
#include <iostream>

#define VERSION "v0.1"
#define APP_NAME "SyncThingy"

class TrayIcon: public QSystemTrayIcon {

private:
    QTimer* timer;
    QProcess* syncthingProcess;

    void setupUi() {
        auto openGitHubAction = new QAction(QString().append(APP_NAME).append(" ").append(VERSION), this);
        auto showBrowserAction = new QAction("Open WebUI", this);
        auto exitAction = new QAction("Exit", this);
        auto menu = new QMenu();

        openGitHubAction->setIcon(QIcon::fromTheme("help-about"));
        showBrowserAction->setIcon(QIcon::fromTheme("help-browser"));
        exitAction->setIcon(QIcon::fromTheme("application-exit"));

        connect(openGitHubAction, &QAction::triggered, this, &TrayIcon::showGitHub);
        connect(showBrowserAction, &QAction::triggered, this, &TrayIcon::showBrowser);
        connect(exitAction, &QAction::triggered, this, &TrayIcon::quit);
        connect(this, &TrayIcon::activated, this, &TrayIcon::handleActivation);

        menu->addAction(openGitHubAction);
        menu->addSeparator();
        menu->addSeparator();
        menu->addAction(showBrowserAction);
        menu->addSeparator();
        menu->addAction(exitAction);

        setContextMenu(menu);
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

    static bool checkSyncthingAvailable() {
        int ret = system("which syncthing");
        return ret == 0;
    }

private slots:
    static void showBrowser() {
        (void) system("xdg-open http://127.0.0.1:8384");
    };

    static void showGitHub() {
        (void) system("xdg-open https://github.com/zocker-160/SyncThingy");
    }

    void handleActivation(QSystemTrayIcon::ActivationReason reason) {
        switch (reason) {
            case QSystemTrayIcon::MiddleClick:
                showBrowser();
        }
    }

    void checkSyncthingRunning() {
        if (not syncthingRunning())
            quit();
    }

public:
    explicit TrayIcon(const QIcon& icon) : QSystemTrayIcon(icon) {
        setupUi();
        show();
        setupProcess();
        setupTimer();
    }

    bool syncthingRunning() {
        return syncthingProcess != nullptr && syncthingProcess->state() == QProcess::Running;
    }

public slots:
    void quit() {
        if (syncthingRunning()) {
            syncthingProcess->terminate();
            syncthingProcess->waitForFinished();
        }

        QString msg = QString("exit code: ").append(QString::number(syncthingProcess->exitCode()));

        if (syncthingProcess->exitCode() == 0)
            showMessage("Syncthing stopped", msg, icon(), 5000);

        QApplication::quit();
    };

};

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QApplication::setApplicationName(APP_NAME);
    QApplication::setApplicationVersion(VERSION);

    QIcon icon;
    const char* flatpakID = std::getenv("FLATPAK_ID");
    if (flatpakID == nullptr) {
        icon = QIcon::fromTheme("syncthing");
    } else {
        std::cout << "running inside Flatpak \n";
        icon = QIcon::fromTheme(flatpakID);
    }
    TrayIcon tray(icon);

    if (not tray.syncthingRunning())
        return 1;

    return QApplication::exec();
}
