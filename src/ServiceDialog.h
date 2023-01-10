#ifndef SYNCTHINGY_SERVICEDIALOG_H
#define SYNCTHINGY_SERVICEDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QSettings>
#include <QFont>
#include <QString>
#include <QColor>
#include <QClipboard>
#include <QUrl>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QIcon>
#include <QPushButton>
#include <QTextBrowser>

#include <QDialogButtonBox>

#include <QDebug>

#include <SingleApplication>

class ServiceDialog : public QDialog {
public:
    explicit ServiceDialog(QWidget* parent = nullptr);

private:
    QLabel* title;
    QTextBrowser* installCommandTextbox;
    QPushButton* copyToClipboardBtn;
    QPushButton* helpINeedMommy;
    QCheckBox* showUninstall;

    void setupUi();
    void copyToClipboard();
    void toggleCommandContent();
};


#endif //SYNCTHINGY_SERVICEDIALOG_H
