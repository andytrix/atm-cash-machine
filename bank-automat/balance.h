#ifndef BALANCEWINDOW_H
#define BALANCEWINDOW_H

#include <QDialog>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QTimer>

namespace Ui {
class BalanceWindow;
}

class BalanceWindow : public QDialog
{
    Q_OBJECT

public:
    explicit BalanceWindow(QWidget *parent = nullptr);
    ~BalanceWindow();

    void setLanguage(const QString &newLanguage);
    void setAuthToken(const QByteArray &token);
    void setIdCard(const QString &id);
    void setCardMode(const QString &mode);

private slots:
    void on_btnBack_clicked();
    void on_btnBack_2_clicked();
    void checkInactivity();

protected:
    void closeEvent(QCloseEvent *) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::BalanceWindow *ui;
    QString selectedLanguage;
    QByteArray authToken;
    QString idCard;
    QString mCardMode;
    QNetworkAccessManager *networkManager;
    QTimer *inactivityTimer;

    void fetchAccountDetails();
    void handleAccountDetails(QNetworkReply *reply);
    void fetchAccountBalance(int accountId, const QString &accountType);
    void handleBalanceResponse(QNetworkReply *reply, const QString &accountType);
};

#endif // BALANCEWINDOW_H
