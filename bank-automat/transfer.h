#ifndef TRANSFERWINDOW_H
#define TRANSFERWINDOW_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

namespace Ui {
class TransferWindow;
}

class TransferWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TransferWindow(QWidget *parent = nullptr);
    ~TransferWindow();

    void setIdcard(const QString &idcard);
    void setCardMode(const QString &mode);
    void setMyToken(const QByteArray &token);
    void setLanguage(const QString &newLanguage);

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void on_buttonTransfer_clicked();
    void on_btnBack_clicked();
    void on_btnBack_2_clicked();
    void onDigitButtonClicked();
    void checkInactivity();
    void on_buttonDebit_clicked();
    void on_buttonCredit_clicked();

private:
    void updateLanguage();
    QString translateMsg(const QString &englishMsg);

    void fetchSenderAccountDetails(double amount, int receiverCardId);
    void onFetchSenderAccountDetailsFinished(QNetworkReply *reply, double amount, int receiverCardId);
    void processSenderWithdrawal(int accountId, const QString &accountType, double amount, int receiverCardId);
    void onSenderWithdrawalResponse(QNetworkReply *reply, double amount, int receiverCardId);

    void fetchReceiverAccountDetails(double amount, int receiverCardId);
    void onFetchReceiverAccountDetailsFinished(QNetworkReply *reply, double amount);
    void processReceiverDeposit(int recAccountId, double amount);
    void onReceiverDepositResponse(QNetworkReply *reply, double amount);
    void logTransaction(int accountId, const QString &type, double amount, const QString &desc);
    void rollbackSenderWithdrawal(int accountId, double amount, const QString &desc);

    Ui::TransferWindow *ui;

    QTimer *inactivityTimer;
    QNetworkAccessManager *networkManager;

    QString mIdcard;
    QString mCardMode;
    QByteArray mToken;
    QString selectedLanguage;

    int senderAccountId = -1;
    int receiverAccountId = -1;
    int mReceiverDebitId = -1;
    int mReceiverCreditId = -1;
    double mReceiverAmount = 0.0;
    bool mReceiverChoiceActive = false;
};

#endif // TRANSFERWINDOW_H
