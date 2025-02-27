#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QTimer>
#include "mainwindow.h"

namespace Ui {
class Login;
}

class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(MainWindow *mainWin, QWidget *parent = nullptr);
    ~Login();

private slots:
    void checkInactivity();
    void on_btnLogin_clicked();
    void LoginSlot(QNetworkReply *reply);
    void cardSlot(QNetworkReply *reply);
    void on_btnLangFI_clicked();
    void on_btnLangSWE_clicked();
    void on_btnLangENG_clicked();
    void on_btn_Stop_clicked();
    void onDigitButtonClicked();

protected:
    void closeEvent(QCloseEvent *) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::Login *ui;
    QTimer *inactivityTimer;
    QNetworkAccessManager *postManager;
    QNetworkReply *reply;
    QByteArray response_data;
    QString selectedLanguage = "FI";
    void updateLanguage();
    bool timerLocked = false;
    MainWindow *mainWindow;
    QString myToken;
    QNetworkAccessManager *getCardManager;
};

#endif // LOGIN_H
