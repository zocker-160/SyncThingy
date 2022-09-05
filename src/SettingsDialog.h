#ifndef SYNCTHINGY_SETTINGSDIALOG_H
#define SYNCTHINGY_SETTINGSDIALOG_H

#include <QDebug>
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

#include <QDialogButtonBox>

#include "Constants.h"

class SettingsDialog : public QDialog {

public:
    explicit SettingsDialog(QSettings& settings, const QIcon& icon);

private:
    QSettings& settings;
    QLineEdit* urlText;
    QComboBox* iconBox;
    QCheckBox* autostartBox;
    QCheckBox* notificationBox;

    void setupUi(const QIcon& icon);
    void loadSettings();
    void saveSettings();

    void accept() override;
};


#endif //SYNCTHINGY_SETTINGSDIALOG_H
