#ifndef CUSTOMERDATA_H
#define CUSTOMERDATA_H

#include <QDialog>
#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QFileDialog>
#include <QHttpMultiPart>
#include <QTimer>

namespace Ui {
class CustomerData;
}

class CustomerData : public QDialog
{
    Q_OBJECT

public:
    explicit CustomerData(QWidget *parent = nullptr);
    ~CustomerData();
    void setIdcard(const QString &newIdcard);
    void setMyToken(const QByteArray &newMyToken);
    void setLanguage(const QString &newLanguage);

private slots:
    void showDataSlot(QNetworkReply *reply);
    void on_btnBack_clicked();
    void on_btnBack_2_clicked();
    void onThumbnailDownloaded(QNetworkReply *reply);
    void onUploadThumbnailFinished(QNetworkReply *reply);
    void onBtnUploadThumbnailClicked();
    void onDigitButtonClicked();
    void on_btnPIN_clicked();
    void on_txtChangePIN_textChanged(const QString &text);
    void changePIN(const QString &oldPin, const QString &newPin);
    void onChangePINFinished(QNetworkReply *reply);
    void verifyOldPIN(const QString &oldPin);
    void onVerifyOldPinFinished(QNetworkReply *reply);
    void checkInactivity();

protected:
    void showEvent(QShowEvent *) override;
    void closeEvent(QCloseEvent *) override;
    void hideEvent(QHideEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    enum PinChangeState {
        Idle,
        EnterOld,
        EnterNew,
        ConfirmNew
    };

    PinChangeState currentPinState;
    QString candidateNewPIN;
    QString verifiedOldPIN;

    Ui::CustomerData *ui;
    QString idcard;
    QByteArray myToken;
    QString selectedLanguage;

    QNetworkAccessManager *dataManager;
    QNetworkReply *reply;
    QByteArray response_data;
    void updateLanguage();

    int customerId;
    QNetworkAccessManager *thumbnailManager;
    QNetworkAccessManager *uploadManager;
    QNetworkAccessManager *pinChangeManager;
    QTimer *inactivityTimer;

    void loadUserThumbnail(int userId);
    void uploadNewThumbnail(int userId, QString filePath);
};

#endif // CUSTOMERDATA_H
