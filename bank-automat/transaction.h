#ifndef TRANSACTIONWINDOW_H
#define TRANSACTIONWINDOW_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QCloseEvent>
#include <QTableWidget>
#include <QTimer>

namespace Ui {
class TransactionWindow;
}

class TransactionWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TransactionWindow(QWidget *parent = nullptr);
    ~TransactionWindow();
    void setLanguage(const QString &newLanguage);
    void fetchTransactions(const QString &idcard, const QString &cardMode, const QByteArray &token);
    void fetchTransactionsByAccount(int accountId, const QByteArray &token);
    void checkInactivity();

private slots:
    void onTransactionDataReceived(QNetworkReply *reply);
    void onCardAccountReceived(QNetworkReply *reply, const QString &idcard, const QString &cardMode, const QByteArray &token);
    void on_btnNext_clicked();
    void on_btnPrev_clicked();
    void on_btnBack_clicked();
    void on_btnBack_2_clicked();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::TransactionWindow *ui;
    QString selectedLanguage;
    QNetworkAccessManager *networkManager;
    QList<QJsonObject> transactionsList;
    int currentPage;
    const int itemsPerPage = 10;
    QTimer *inactivityTimer;
    void updateLanguage();
    void updateTransactionList();
};

#endif // TRANSACTIONWINDOW_H
