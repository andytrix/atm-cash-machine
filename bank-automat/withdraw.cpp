#include "withdraw.h"
#include "ui_withdraw.h"
#include "environment.h"
#include "mainmenu.h"
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QCloseEvent>
#include <QApplication>
#include <QPushButton>

WithdrawWindow::WithdrawWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WithdrawWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("ATM");

    inactivityTimer = new QTimer(this);
    inactivityTimer->setInterval(10000); // 10 sec
    connect(inactivityTimer, &QTimer::timeout, this, &WithdrawWindow::checkInactivity);
    qApp->installEventFilter(this);

    networkManager = new QNetworkAccessManager(this);

    for (int i = 0; i < 12; ++i) {
        QString buttonName = "btn_" + QString::number(i);
        QPushButton *button = this->findChild<QPushButton *>(buttonName);
        if (button) {
            connect(button, &QPushButton::clicked, this, &WithdrawWindow::onDigitButtonClicked);
        }
    }
    QPushButton *deleteButton = this->findChild<QPushButton *>("btn_14");
    if (deleteButton) {
        connect(deleteButton, &QPushButton::clicked, this, &WithdrawWindow::onDigitButtonClicked);
    }

    ui->txtOtherAmount->setVisible(false);
}

WithdrawWindow::~WithdrawWindow()
{
    delete ui;
}

void WithdrawWindow::showEvent(QShowEvent *event)
{
    inactivityTimer->start(10000);
    QDialog::showEvent(event);
}

void WithdrawWindow::hideEvent(QHideEvent *event)
{
    inactivityTimer->stop();
    inactivityTimer->setInterval(10000);
    QDialog::hideEvent(event);
}

bool WithdrawWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress)
    {
        inactivityTimer->start(10000);
    }
    return QDialog::eventFilter(obj, event);
}

void WithdrawWindow::checkInactivity()
{
    if (!this->isVisible())
    {
        inactivityTimer->stop();
        return;
    }

    this->hide();
    QWidget *parentWidget = qobject_cast<QWidget*>(parent());
    if (parentWidget) {
        parentWidget->setGeometry(this->geometry());
        parentWidget->show();
    }
}

void WithdrawWindow::setIdcard(const QString &idcard)
{
    mIdcard = idcard;
}

void WithdrawWindow::setCardMode(const QString &mode)
{
    mCardMode = mode;
}

void WithdrawWindow::setMyToken(const QByteArray &token)
{
    mToken = token;
}

void WithdrawWindow::setLanguage(const QString &newLanguage)
{
    selectedLanguage = newLanguage;
    updateLanguage();
}

void WithdrawWindow::updateLanguage()
{
    if (selectedLanguage == "FI") {
        ui->btnBack->setText("STOP");
        ui->txtBack->setText("Takaisin");
        ui->txtOut_x->setText("Muu summa");
    }

    else if (selectedLanguage == "SWE") {
        ui->btnBack->setText("STOP");
        ui->txtBack->setText("Tillbaka");
        ui->txtOut_x->setText("Annan summa");
    }
    else if (selectedLanguage == "ENG") {
        ui->btnBack->setText("STOP");
        ui->txtBack->setText("Back");
        ui->txtOut_x->setText("Other amount");
    }
}

void WithdrawWindow::hideCustomAmountInput()
{
    ui->txtOtherAmount->setVisible(false);
}

void WithdrawWindow::on_btnBack_clicked()
{
    this->hide();
    QWidget *parentWidget = qobject_cast<QWidget*>(parent());
    if (parentWidget) {
        parentWidget->setGeometry(this->geometry());
        parentWidget->show();
    }
}

void WithdrawWindow::on_btnBack_2_clicked()
{
    this->hide();
    QWidget *parentWidget = qobject_cast<QWidget*>(parent());
    if (parentWidget) {
        parentWidget->setGeometry(this->geometry());
        parentWidget->show();
    }
}

void WithdrawWindow::closeEvent(QCloseEvent *)
{
    QApplication::quit();
}

void WithdrawWindow::on_buttonOut_20_clicked()
{
    ui->txtINFO->clear();
    hideCustomAmountInput();
    int amount = 20;
    fetchAccountDetails(amount);
}

void WithdrawWindow::on_buttonOut_40_clicked()
{
    ui->txtINFO->clear();
    hideCustomAmountInput();
    int amount = 40;
    fetchAccountDetails(amount);
}

void WithdrawWindow::on_buttonOut_50_clicked()
{
    ui->txtINFO->clear();
    hideCustomAmountInput();
    int amount = 50;
    fetchAccountDetails(amount);
}

void WithdrawWindow::on_buttonOut_80_clicked()
{
    ui->txtINFO->clear();
    hideCustomAmountInput();
    int amount = 80;
    fetchAccountDetails(amount);
}

void WithdrawWindow::on_buttonOut_100_clicked()
{
    ui->txtINFO->clear();
    hideCustomAmountInput();
    int amount = 100;
    fetchAccountDetails(amount);
}

void WithdrawWindow::on_buttonOut_x_clicked()
{
    ui->txtINFO->clear();
    ui->txtOtherAmount->clear();
    ui->txtOtherAmount->setVisible(true);
    ui->txtOtherAmount->setFocus();
}

void WithdrawWindow::on_btnOK_clicked()
{
    if (!ui->txtOtherAmount->isVisible()) {
        return;
    }

    bool ok;
    int customAmount = ui->txtOtherAmount->text().toInt(&ok);

    if (!ok) {
        if (selectedLanguage == "FI") {
            ui->txtINFO->setText("Virheellinen summa");
        } else if (selectedLanguage == "SWE") {
            ui->txtINFO->setText("Ogiltigt belopp");
        } else if (selectedLanguage == "ENG") {
            ui->txtINFO->setText("Invalid amount");
        } else {
            ui->txtINFO->setText("Virheellinen summa");
        }
        return;
    }

    if (customAmount % 5 != 0) {
        if (selectedLanguage == "FI") {
            ui->txtINFO->setText("Vain summat 5 € välein");
        } else if (selectedLanguage == "SWE") {
            ui->txtINFO->setText("Endast belopp i 5 € intervall");
        } else if (selectedLanguage == "ENG") {
            ui->txtINFO->setText("Only amounts in 5 € increments");
        } else {
            ui->txtINFO->setText("Vain summat 5 € välein");
        }
        return;
    }

    hideCustomAmountInput();
    fetchAccountDetails(customAmount);
}

void WithdrawWindow::fetchAccountDetails(int amount)
{
    QString url = Environment::base_url() + "/card_account?idcard=" + mIdcard;
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", mToken);

    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        onFetchAccountDetailsFinished(reply, amount);
    });
}

void WithdrawWindow::onFetchAccountDetailsFinished(QNetworkReply *reply, int amount)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

        if (!jsonDoc.isArray()) {
            ui->txtINFO->setText("Unexpected JSON format from /card_account!");
            reply->deleteLater();
            return;
        }

        QJsonArray arr = jsonDoc.array();
        if (arr.isEmpty()) {
            ui->txtINFO->setText("No account found for this card ID!");
            reply->deleteLater();
            return;
        }

        selectedAccountId = -1;

        for (auto val : arr) {
            QJsonObject obj = val.toObject();
            if (obj["type"].toString() == mCardMode && obj["idcard"].toInt() == mIdcard.toInt()) {
                if (obj["idaccount"].toInt() > selectedAccountId) {
                    selectedAccountId = obj["idaccount"].toInt();
                }
            }
        }

        if (selectedAccountId == -1) {
            ui->txtINFO->setText("No matching account type found (" + mCardMode + ")!");
        } else {
            processWithdrawal(selectedAccountId, mCardMode, amount);
        }

    } else {
        ui->txtINFO->setText("Failed to retrieve card_account: " + reply->errorString());
    }
    reply->deleteLater();
}

void WithdrawWindow::processWithdrawal(int accountId, const QString &accountType, double amount)
{
    QString url = Environment::base_url() + "/account/" + QString::number(accountId);
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", mToken);

    QNetworkReply *getReply = networkManager->get(request);

    connect(getReply, &QNetworkReply::finished, this, [=]() {
        if (getReply->error() == QNetworkReply::NoError) {
            QByteArray responseData = getReply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(responseData);
            QJsonObject obj = doc.object();

            bool success = false;
            QString errorMessage;

            if (accountType == "debit") {
                double currentBalance = obj["debit_balance"].toString().toDouble();
                if (currentBalance >= amount) {
                    obj["debit_balance"] = QString::number(currentBalance - amount, 'f', 2);
                    success = true;
                } else {
                    if (selectedLanguage == "SWE") {
                        errorMessage = "Otillräckliga medel";
                    } else if (selectedLanguage == "ENG") {
                        errorMessage = "Insufficient funds";
                    } else {
                        errorMessage = "Tilillä ei ole katetta";
                    }
                    ui->txtINFO->setText(errorMessage);
                }
            }
            else if (accountType == "credit") {
                double currentBalance = obj["credit_balance"].toString().toDouble();
                double creditLimit = obj["credit_limit"].toString().toDouble();
                double newBalance = currentBalance - amount;

                if (newBalance >= -creditLimit) {
                    obj["credit_balance"] = QString::number(newBalance, 'f', 2);
                    success = true;
                } else {
                    if (selectedLanguage == "SWE") {
                        errorMessage = "Kreditgränsen överskriden";
                    } else if (selectedLanguage == "ENG") {
                        errorMessage = "Credit limit exceeded";
                    } else {
                        errorMessage = "Luottoraja ylittyy";
                    }
                    ui->txtINFO->setText(errorMessage);
                }
            }

            if (success) {
                QJsonDocument updateDoc(obj);
                QByteArray updateData = updateDoc.toJson();

                QNetworkRequest putReq(getReply->url());
                putReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
                putReq.setRawHeader("Authorization", mToken);

                QNetworkReply *putReply = networkManager->sendCustomRequest(putReq, "PUT", updateData);
                connect(putReply, &QNetworkReply::finished, this, [=]() {
                    onWithdrawalResponse(putReply, amount);
                });
            }
        }
        getReply->deleteLater();
    });
}

void WithdrawWindow::onWithdrawalResponse(QNetworkReply *reply, int amount)
{
    QString successMessage, errorMessage;
    if (selectedLanguage == "SWE") {
        successMessage = QString("Uttag lyckades (%1 €)").arg(amount);
        errorMessage = "Uppdatering av uttaget misslyckades: ";
    } else if (selectedLanguage == "ENG") {
        successMessage = QString("Withdrawal successful (%1 €)").arg(amount);
        errorMessage = "Withdrawal update failed: ";
    } else {
        successMessage = QString("Nosto onnistui (%1 €)").arg(amount);
        errorMessage = "Noston päivitys epäonnistui: ";
    }

    if (reply->error() == QNetworkReply::NoError) {
        ui->txtINFO->setText(successMessage);
        logTransaction(amount);
    } else {
        ui->txtINFO->setText(errorMessage + reply->errorString());
    }

    reply->deleteLater();
}

void WithdrawWindow::logTransaction(int amount)
{
    QString url = Environment::base_url() + "/transaction";
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", mToken);

    QJsonObject transactionObj;
    transactionObj["idaccount"] = selectedAccountId;
    transactionObj["type"] = "withdrawal";
    transactionObj["amount"] = amount;
    transactionObj["description"] = "ATM Withdrawal";

    QJsonDocument doc(transactionObj);
    QByteArray jsonData = doc.toJson();

    QNetworkReply *reply = networkManager->post(request, jsonData);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "Transaction logged successfully.";
        } else {
            qDebug() << "Failed to log transaction:" << reply->errorString();
        }
        reply->deleteLater();
    });
}

void WithdrawWindow::onDigitButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button)
        return;

    QString buttonName = button->objectName();

    if (buttonName == "btn_14") {
        if (ui->txtOtherAmount->hasFocus()) {
            QString currentText = ui->txtOtherAmount->text();
            if (!currentText.isEmpty()) {
                currentText.chop(1);
                ui->txtOtherAmount->setText(currentText);
            }
        }
        return;
    }

    QString value;
    if (buttonName == "btn_10") {
        value = "+";
    }
    else if (buttonName == "btn_11") {
        value = "-";
    }
    else if (buttonName.startsWith("btn_")) {
        value = buttonName.right(1);
    }
    else {
        return;
    }

    if (ui->txtOtherAmount->hasFocus()) {
        ui->txtOtherAmount->insert(value);
    }
}
