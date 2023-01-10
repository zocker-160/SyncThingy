#ifndef SYNCTHINGY_SETTINGSDIALOG_H
#define SYNCTHINGY_SETTINGSDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QSettings>
#include <QFont>
#include <QString>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QIcon>
#include <QPushButton>

#include <QDialogButtonBox>

#include <QDebug>

#include "Constants.h"
#include "ServiceDialog.h"

class SettingsDialog : public QDialog {

public:
    explicit SettingsDialog(QSettings& settings, const QIcon& icon, QWidget* parent = nullptr);

private:
    QSettings& settings;
    QLineEdit* urlText;
    QComboBox* iconBox;
    QCheckBox* autostartBox;
    QCheckBox* notificationBox;
    QPushButton* createBGService;

    void setupUi(const QIcon& icon);
    void loadSettings();
    void saveSettings();

    void accept() override;

    void showServiceDialog();
};


#endif //SYNCTHINGY_SETTINGSDIALOG_H
