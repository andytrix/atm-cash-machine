#include "balance.h"
#include "ui_balance.h"
#include "mainmenu.h"
#include "environment.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

BalanceWindow::BalanceWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BalanceWindow),
    networkManager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);
    this->setWindowTitle("ATM");

    inactivityTimer = new QTimer(this);
    inactivityTimer->setInterval(10000); // 10 sec
    connect(inactivityTimer, &QTimer::timeout, this, &BalanceWindow::checkInactivity);

    qApp->installEventFilter(this);
}

BalanceWindow::~BalanceWindow()
{
    delete ui;
}

void BalanceWindow::showEvent(QShowEvent *event)
{
    inactivityTimer->start(10000);
    QDialog::showEvent(event);
}

void BalanceWindow::hideEvent(QHideEvent *event)
{
    inactivityTimer->stop();
    inactivityTimer->setInterval(10000);
    QDialog::hideEvent(event);
}

bool BalanceWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress)
    {
        inactivityTimer->start(10000);
    }
    return QDialog::eventFilter(obj, event);
}

void BalanceWindow::checkInactivity()
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

void BalanceWindow::setAuthToken(const QByteArray &token)
{
    authToken = token;
}

void BalanceWindow::setCardMode(const QString &mode)
{
    mCardMode = mode;
}


void BalanceWindow::setIdCard(const QString &id)
{
    idCard = id;
    fetchAccountDetails();
}

void BalanceWindow::fetchAccountDetails()
{
    QString url = Environment::base_url() + "/card_account?idcard=" + idCard;
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", authToken);

    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        handleAccountDetails(reply);
    });
}

void BalanceWindow::handleAccountDetails(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        qDebug() << "Server response:" << responseData;

        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

        if (!jsonDoc.isArray()) {
            qDebug() << "Invalid JSON format from /card_account endpoint!";
            reply->deleteLater();
            return;
        }

        QJsonArray arr = jsonDoc.array();
        if (arr.isEmpty()) {
            qDebug() << "No accounts found for the card!";
            reply->deleteLater();
            return;
        }

        QList<int> accountIds;
        QList<QString> accountTypes;

        qDebug() << "Logged-in user's card (idCard):" << idCard;

        for (const QJsonValue &val : arr) {
            QJsonObject obj = val.toObject();
            int jsonIdCard = obj["idcard"].toInt();
            int accountId = obj["idaccount"].toInt();
            QString accountType = obj["type"].toString(); // "debit" or "credit"

            if (jsonIdCard == idCard.toInt()) {
                qDebug() << "Account belongs to current card -> Type:" << accountType << ", ID:" << accountId;

                accountIds.append(accountId);
                accountTypes.append(accountType);
            } else {
                qDebug() << "Skipping account from different card (idcard:" << jsonIdCard << ")";
            }
        }

        if (accountIds.isEmpty()) {
            qDebug() << "No accounts belonging to the current card found!";
            reply->deleteLater();
            return;
        }

        if (accountTypes.size() == 1) {
            qDebug() << "Only one account on the card, automatically fetching balance.";
            fetchAccountBalance(accountIds.first(), accountTypes.first());
            return;
        }

        int debitIndex = accountTypes.indexOf("debit");
        int creditIndex = accountTypes.indexOf("credit");

        if (debitIndex != -1) {
            qDebug() << "Fetching debit account balance, ID:" << accountIds[debitIndex];
            fetchAccountBalance(accountIds[debitIndex], "debit");
        }

        if (creditIndex != -1) {
            qDebug() << "Fetching credit account balance, ID:" << accountIds[creditIndex];
            fetchAccountBalance(accountIds[creditIndex], "credit");
        }
    } else {
        qDebug() << "Error fetching account details:" << reply->errorString();
    }
    reply->deleteLater();
}

void BalanceWindow::fetchAccountBalance(int accountId, const QString &accountType)
{
    QString url = Environment::base_url() + "/account/" + QString::number(accountId);
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", authToken);

    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        handleBalanceResponse(reply, accountType);
    });
}

void BalanceWindow::handleBalanceResponse(QNetworkReply *reply, const QString &accountType)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        if (!jsonDoc.isObject()) {
            qDebug() << "Invalid JSON from /account endpoint!";
            reply->deleteLater();
            return;
        }
        QJsonObject obj = jsonDoc.object();

        if (accountType == "debit" && obj.contains("debit_balance")) {
            double debitBalance = 0.0;
            QJsonValue val = obj["debit_balance"];
            if (val.isDouble()) {
                debitBalance = val.toDouble();
            } else if (val.isString()) {
                debitBalance = val.toString().toDouble();
            }
            ui->balanceDebit->setText(QString::number(debitBalance));
        } else if (accountType == "credit" && obj.contains("credit_balance")) {
            double creditBalance = 0.0;
            QJsonValue val = obj["credit_balance"];
            if (val.isDouble()) {
                creditBalance = val.toDouble();
            } else if (val.isString()) {
                creditBalance = val.toString().toDouble();
            }
            ui->balanceCredit->setText(QString::number(creditBalance));

            if (obj.contains("credit_limit")) {
                double creditLimit = 0.0;
                QJsonValue valLimit = obj["credit_limit"];
                if (valLimit.isDouble()) {
                    creditLimit = valLimit.toDouble();
                } else if (valLimit.isString()) {
                    creditLimit = valLimit.toString().toDouble();
                }
                ui->balanceCreditLimit->setText(QString::number(creditLimit));
            }
        } else {
            qDebug() << "Expected fields not found in JSON response!";
        }
    } else {
        qDebug() << "Error fetching account balance:" << reply->errorString();
    }
    reply->deleteLater();
}

void BalanceWindow::on_btnBack_clicked()
{
    this->hide();
    QWidget *parentWidget = qobject_cast<QWidget*>(parent());
    if (parentWidget) {
        parentWidget->setGeometry(this->geometry());
        parentWidget->show();
    }
}

void BalanceWindow::on_btnBack_2_clicked()
{
    this->hide();
    QWidget *parentWidget = qobject_cast<QWidget*>(parent());
    if (parentWidget) {
        parentWidget->setGeometry(this->geometry());
        parentWidget->show();
    }
}

void BalanceWindow::setLanguage(const QString &newLanguage)
{
    selectedLanguage = newLanguage;
    if (selectedLanguage == "FI") {
        ui->txtBack->setText("Takaisin");
        ui->txtDebit->setText("Debit:");
        ui->txtCredit->setText("Luottosaldo:");
        ui->txtCredit_2->setText("Luottoraja:");
    }
    else if (selectedLanguage == "SWE") {
        ui->txtBack->setText("Tillbaka");
        ui->txtDebit->setText("Debet:");
        ui->txtCredit->setText("Kreditsaldo:");
        ui->txtCredit_2->setText("Kreditgräns:");
    }
    else if (selectedLanguage == "ENG") {
        ui->txtBack->setText("Back");
        ui->txtDebit->setText("Debit:");
        ui->txtCredit->setText("Credit balance:");
        ui->txtCredit_2->setText("Credit limit:");
    }
}

void BalanceWindow::closeEvent(QCloseEvent *)
{
    QApplication::quit();
}
