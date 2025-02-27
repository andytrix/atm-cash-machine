#include "transfer.h"
#include "ui_transfer.h"
#include "environment.h"

#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QCloseEvent>
#include <QApplication>
#include <QPushButton>

TransferWindow::TransferWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransferWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("ATM");

    inactivityTimer = new QTimer(this);
    inactivityTimer->setInterval(10000);
    connect(inactivityTimer, &QTimer::timeout, this, &TransferWindow::checkInactivity);
    qApp->installEventFilter(this);

    networkManager = new QNetworkAccessManager(this);

    ui->buttonDebit->setEnabled(true);
    ui->buttonCredit->setEnabled(true);

    mReceiverChoiceActive = false;

    ui->txtDebit->setVisible(false);
    ui->txtCredit->setVisible(false);

    connect(ui->buttonDebit, &QPushButton::clicked, this, &TransferWindow::on_buttonDebit_clicked);
    connect(ui->buttonCredit, &QPushButton::clicked, this, &TransferWindow::on_buttonCredit_clicked);

    for (int i = 0; i < 12; ++i) {
        QString buttonName = "btn_" + QString::number(i);
        if (QPushButton *b = this->findChild<QPushButton *>(buttonName)) {
            connect(b, &QPushButton::clicked, this, &TransferWindow::onDigitButtonClicked);
        }
    }

    if (QPushButton *deleteButton = this->findChild<QPushButton *>("btn_14")) {
        connect(deleteButton, &QPushButton::clicked, this, &TransferWindow::onDigitButtonClicked);
    }

    ui->txtAccount->setFocus();
}

TransferWindow::~TransferWindow()
{
    delete ui;
}

void TransferWindow::showEvent(QShowEvent *event)
{
    inactivityTimer->start(10000);
    QDialog::showEvent(event);
}

void TransferWindow::hideEvent(QHideEvent *event)
{
    inactivityTimer->stop();
    inactivityTimer->setInterval(10000);
    QDialog::hideEvent(event);
}

bool TransferWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseMove ||
        event->type() == QEvent::MouseButtonPress)
    {
        inactivityTimer->start(10000);
    }
    return QDialog::eventFilter(obj, event);
}

void TransferWindow::checkInactivity()
{
    if (!this->isVisible()) {
        inactivityTimer->stop();
        return;
    }
    this->hide();
    if (QWidget *p = qobject_cast<QWidget*>(parent())) {
        p->setGeometry(this->geometry());
        p->show();
    }
}

void TransferWindow::closeEvent(QCloseEvent *)
{
    QApplication::quit();
}

void TransferWindow::setIdcard(const QString &idcard)
{
    mIdcard = idcard;
}

void TransferWindow::setCardMode(const QString &mode)
{
    mCardMode = mode;
}

void TransferWindow::setMyToken(const QByteArray &token)
{
    mToken = token;
}

void TransferWindow::setLanguage(const QString &newLanguage)
{
    selectedLanguage = newLanguage;
    updateLanguage();
}

void TransferWindow::updateLanguage()
{
    if (selectedLanguage == "SWE") {
        ui->txtBack->setText("Tillbaka");
        ui->txtTransferGO->setText("Överför");
        ui->txtDebit->setText("Debet");
        ui->txtCredit->setText("Kredit");
        ui->txtInfoAccount->setText("Till kortet");
        ui->txtInfoAmount->setText("Summa");
    }
    else if (selectedLanguage == "ENG") {
        ui->txtBack->setText("Back");
        ui->txtTransferGO->setText("Transfer");
        ui->txtDebit->setText("Debit");
        ui->txtCredit->setText("Credit");
        ui->txtInfoAccount->setText("Send to card");
        ui->txtInfoAmount->setText("Amount");
    }
    else {
        ui->txtBack->setText("Takaisin");
        ui->txtTransferGO->setText("Siirrä");
        ui->txtDebit->setText("Debit");
        ui->txtCredit->setText("Credit");
        ui->txtInfoAccount->setText("Siirrä kortille");
        ui->txtInfoAmount->setText("Summa");
    }
}

void TransferWindow::on_buttonTransfer_clicked()
{
    mReceiverChoiceActive = false;
    ui->txtDebit->setVisible(false);
    ui->txtCredit->setVisible(false);

    ui->txtINFO->clear();

    bool ok;
    int receiverCardId = ui->txtAccount->text().toInt(&ok);
    if (!ok || receiverCardId <= 0) {
        if (selectedLanguage == "SWE") {
            ui->txtINFO->setText("Ogiltigt målkort-ID!");
        } else if (selectedLanguage == "ENG") {
            ui->txtINFO->setText("Invalid target card ID!");
        } else {
            ui->txtINFO->setText("Virheellinen kohdekortin ID!");
        }
        return;
    }

    double transferAmount = ui->txtAmount->text().toDouble(&ok);
    if (!ok || transferAmount <= 0) {
        if (selectedLanguage == "SWE") {
            ui->txtINFO->setText("Ogiltigt belopp!");
        } else if (selectedLanguage == "ENG") {
            ui->txtINFO->setText("Invalid amount!");
        } else {
            ui->txtINFO->setText("Virheellinen summa!");
        }

        return;
    }

    fetchSenderAccountDetails(transferAmount, receiverCardId);
}

void TransferWindow::fetchSenderAccountDetails(double amount, int receiverCardId)
{
    QString url = Environment::base_url() + "/card_account?idcard=" + mIdcard;
    QNetworkRequest req(url);
    req.setRawHeader("Authorization", mToken);

    QNetworkReply *reply = networkManager->get(req);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        onFetchSenderAccountDetailsFinished(reply, amount, receiverCardId);
    });
}

void TransferWindow::onFetchSenderAccountDetailsFinished(QNetworkReply *reply, double amount, int receiverCardId)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray resp = reply->readAll();
        reply->deleteLater();

        QJsonDocument doc = QJsonDocument::fromJson(resp);
        if (!doc.isArray()) {
            ui->txtINFO->setText("Invalid JSON /card_account!");
            return;
        }
        QJsonArray arr = doc.array();
        if (arr.isEmpty()) {
            ui->txtINFO->setText("No accounts found for this card ID!");
            return;
        }

        senderAccountId = -1;
        QString chosenType;

        QString primaryType, fallbackType;
        if (mCardMode == "credit") {
            primaryType = "credit";
            fallbackType = "debit";
        }
        else if (mCardMode == "debit") {
            primaryType = "debit";
            fallbackType = "credit";
        }
        else {
            primaryType = "debit";
            fallbackType = "credit";
        }

        int primaryId = -1;
        int fbId = -1;

        for (auto val : arr) {
            QJsonObject obj = val.toObject();
            if (obj["idcard"].toInt() != mIdcard.toInt()) {
                continue;
            }
            QString rowType = obj["type"].toString();
            int candidateId = obj["idaccount"].toInt();

            if (rowType == primaryType && primaryId == -1) {
                primaryId = candidateId;
            }
            else if (rowType == fallbackType && fbId == -1) {
                fbId = candidateId;
            }
        }

        if (primaryId != -1) {
            senderAccountId = primaryId;
            chosenType = primaryType;
        }
        else if (fbId != -1) {
            senderAccountId = fbId;
            chosenType = fallbackType;
        }
        else {
            ui->txtINFO->setText(
                (selectedLanguage == "SWE") ?
                    "Ingen kontotyp hittades\n('" + primaryType + "' eller '" + fallbackType + "')!" :
                    (selectedLanguage == "ENG") ?
                        "No account type found\n('" + primaryType + "' or '" + fallbackType + "')!" :
                        "Ei löytynyt tilityyppiä\n('" + primaryType + "' tai '" + fallbackType + "')!"
                );
            return;
        }

        processSenderWithdrawal(senderAccountId, chosenType, amount, receiverCardId);

    } else {
        ui->txtINFO->setText(
            (selectedLanguage == "SWE") ?
                "Kortutgifter kunde inte hämtas: " + reply->errorString() :
                (selectedLanguage == "ENG") ?
                    "Failed to retrieve card details: " + reply->errorString() :
                    "Korttitietoja ei saatu: " + reply->errorString()
            );
        reply->deleteLater();
    }
}

void TransferWindow::processSenderWithdrawal(int accountId, const QString &accountType, double amount, int receiverCardId)
{
    QString url = Environment::base_url() + "/account/" + QString::number(accountId);
    QNetworkRequest req(url);
    req.setRawHeader("Authorization", mToken);

    QNetworkReply *r = networkManager->get(req);
    connect(r, &QNetworkReply::finished, this, [=]() {
        if (r->error() == QNetworkReply::NoError) {
            QByteArray data = r->readAll();
            r->deleteLater();

            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonObject obj = doc.object();

            bool success = false;
            QString msg;
            if (accountType == "debit") {
                double curBal = obj["debit_balance"].toString().toDouble();
                if (curBal >= amount) {
                    obj["debit_balance"] = QString::number(curBal - amount, 'f', 2);
                    success = true;
                } else {
                    msg = translateMsg("Insufficient funds");
                }
            }
            else {
                double curBal = obj["credit_balance"].toString().toDouble();
                double limit = obj["credit_limit"].toString().toDouble();
                double newBal = curBal - amount;
                if (newBal >= -limit) {
                    obj["credit_balance"] = QString::number(newBal, 'f', 2);
                    success = true;
                } else {
                    msg = translateMsg("Credit limit exceeded");
                }
            }

            if (!success) {
                ui->txtINFO->setText(msg);
                return;
            }

            QJsonDocument upDoc(obj);
            QByteArray upData = upDoc.toJson();

            QNetworkRequest putReq(url);
            putReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            putReq.setRawHeader("Authorization", mToken);

            QNetworkReply *putReply = networkManager->sendCustomRequest(putReq, "PUT", upData);
            connect(putReply, &QNetworkReply::finished, this, [=]() {
                onSenderWithdrawalResponse(putReply, amount, receiverCardId);
            });
        }
        else {
            ui->txtINFO->setText("Account retrieval failed: " + r->errorString());
            r->deleteLater();
        }
    });
}

void TransferWindow::onSenderWithdrawalResponse(QNetworkReply *reply, double amount, int receiverCardId)
{
    if (reply->error() == QNetworkReply::NoError) {
        reply->deleteLater();
        fetchReceiverAccountDetails(amount, receiverCardId);
    }
    else {
        ui->txtINFO->setText(translateMsg("Transfer update failed: ") + reply->errorString());
        reply->deleteLater();
    }
}

void TransferWindow::fetchReceiverAccountDetails(double amount, int receiverCardId)
{
    mReceiverDebitId = -1;
    mReceiverCreditId = -1;
    mReceiverAmount = amount;

    QString url = Environment::base_url() + "/card_account?idcard=" + QString::number(receiverCardId);
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", mToken);

    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        onFetchReceiverAccountDetailsFinished(reply, amount);
    });
}

void TransferWindow::onFetchReceiverAccountDetailsFinished(QNetworkReply *reply, double amount)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray d = reply->readAll();
        reply->deleteLater();

        QJsonDocument doc = QJsonDocument::fromJson(d);
        if (!doc.isArray()) {
            ui->txtINFO->setText("Invalid JSON /card_account!");

            rollbackSenderWithdrawal(senderAccountId, amount, "Rollback: Invalid JSON from recipient info");
            return;
        }
        QJsonArray arr = doc.array();
        if (arr.isEmpty()) {
            ui->txtINFO->setText("No accounts found for the recipient!");

            rollbackSenderWithdrawal(senderAccountId, amount, "Rollback: recipient not found");
            return;
        }

        for (auto val : arr) {
            QJsonObject obj = val.toObject();
            if (obj["idcard"].toInt() != ui->txtAccount->text().toInt()) {
                continue;
            }
            QString t = obj["type"].toString();
            int candidate = obj["idaccount"].toInt();

            if (t == "debit" && mReceiverDebitId == -1) {
                mReceiverDebitId = candidate;
            }
            else if (t == "credit" && mReceiverCreditId == -1) {
                mReceiverCreditId = candidate;
            }
        }
        if (mReceiverDebitId != -1 && mReceiverCreditId != -1) {
            if (selectedLanguage == "SWE") {
                ui->txtINFO->setText("Kortet har både ett\ndebit- och ett kreditkonto\nVälj vilket du vill sätta in på");
            } else if (selectedLanguage == "ENG") {
                ui->txtINFO->setText("The card has both\na debit and a credit account\nSelect where to deposit");
            } else {
                ui->txtINFO->setText("Kortilla on sekä debit- että credit-tili\nValitse kumpaan talletetaan");
            }

            ui->txtDebit->setVisible(true);
            ui->txtCredit->setVisible(true);

            mReceiverChoiceActive = true;
        }
        else if (mReceiverDebitId != -1) {
            if (selectedLanguage == "SWE") {
                ui->txtINFO->setText("Endast debitkonto\nInsättning görs direkt");
            } else if (selectedLanguage == "ENG") {
                ui->txtINFO->setText("Only a debit account\nDeposit will be made directly");
            } else {
                ui->txtINFO->setText("Vain debit-tili\nTalletetaan suoraan");
            }
            processReceiverDeposit(mReceiverDebitId, amount);
        }
        else if (mReceiverCreditId != -1) {
            if (selectedLanguage == "SWE") {
                ui->txtINFO->setText("Endast kreditkonto\nInsättning görs direkt");
            } else if (selectedLanguage == "ENG") {
                ui->txtINFO->setText("Only a credit account\nDeposit will be made directly");
            } else {
                ui->txtINFO->setText("Vain credit-tili\nTalletetaan suoraan");
            }
            processReceiverDeposit(mReceiverCreditId, amount);
        }
        else {
            if (selectedLanguage == "SWE") {
                ui->txtINFO->setText("Inga debit- eller kreditkonton\nför mottagaren!");
            } else if (selectedLanguage == "ENG") {
                ui->txtINFO->setText("No debit or credit accounts\nfor the recipient!");
            } else {
                ui->txtINFO->setText("Ei debit- tai credit-tilejä\nvastaanottajalla!");
            }

            rollbackSenderWithdrawal(senderAccountId, amount, "Rollback: no valid deposit account");
        }
    }
    else {
        if (selectedLanguage == "SWE") {
            ui->txtINFO->setText("Mottagarens kontoinformation kunde\ninte hämtas: " + reply->errorString());
        } else if (selectedLanguage == "ENG") {
            ui->txtINFO->setText("Failed to retrieve recipient's\naccount information: " + reply->errorString());
        } else {
            ui->txtINFO->setText("Vastaanottajan tilitietoja ei saatu:\n" + reply->errorString());
        }
        reply->deleteLater();

        rollbackSenderWithdrawal(senderAccountId, amount, "Rollback: network error fetching recipient");
    }
}

void TransferWindow::on_buttonDebit_clicked()
{
    if (!mReceiverChoiceActive) {
        return;
    }

    mReceiverChoiceActive = false;
    ui->txtDebit->setVisible(false);
    ui->txtCredit->setVisible(false);

    if (mReceiverDebitId == -1) {
        if (selectedLanguage == "SWE") {
            ui->txtINFO->setText("Ingen debitkonto\nför mottagaren!");
        } else if (selectedLanguage == "ENG") {
            ui->txtINFO->setText("No debit account\nfor the recipient!");
        } else {
            ui->txtINFO->setText("Ei debit-tiliä\nvastaanottajalla!");
        }
        return;
    }
    processReceiverDeposit(mReceiverDebitId, mReceiverAmount);
}

void TransferWindow::on_buttonCredit_clicked()
{
    if (!mReceiverChoiceActive) {
        return;
    }
    mReceiverChoiceActive = false;
    ui->txtDebit->setVisible(false);
    ui->txtCredit->setVisible(false);

    if (mReceiverCreditId == -1) {
        if (selectedLanguage == "SWE") {
            ui->txtINFO->setText("Ingen kreditkonto\nför mottagaren!");
        } else if (selectedLanguage == "ENG") {
            ui->txtINFO->setText("No credit account\nfor the recipient!");
        } else {
            ui->txtINFO->setText("Ei credit-tiliä\nvastaanottajalla!");
        }
        return;
    }
    processReceiverDeposit(mReceiverCreditId, mReceiverAmount);
}

void TransferWindow::processReceiverDeposit(int recAccountId, double amount)
{
    receiverAccountId = recAccountId;

    QString url = Environment::base_url() + "/account/" + QString::number(recAccountId);
    QNetworkRequest req(url);
    req.setRawHeader("Authorization", mToken);

    QNetworkReply *r = networkManager->get(req);
    connect(r, &QNetworkReply::finished, this, [=]() {
        if (r->error() == QNetworkReply::NoError) {
            QByteArray data = r->readAll();
            r->deleteLater();

            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonObject obj = doc.object();

            QString at = obj["type"].toString();
            if (at == "debit") {
                double cb = obj["debit_balance"].toString().toDouble();
                obj["debit_balance"] = QString::number(cb + amount, 'f', 2);
            }
            else {
                double cb = obj["credit_balance"].toString().toDouble();
                obj["credit_balance"] = QString::number(cb + amount, 'f', 2);
            }

            QNetworkRequest putReq(url);
            putReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            putReq.setRawHeader("Authorization", mToken);

            QJsonDocument upDoc(obj);
            QByteArray upData = upDoc.toJson();

            QNetworkReply *putReply = networkManager->sendCustomRequest(putReq, "PUT", upData);
            connect(putReply, &QNetworkReply::finished, this, [=]() {
                onReceiverDepositResponse(putReply, amount);
            });
        }else {
            if (selectedLanguage == "SWE") {
                ui->txtINFO->setText("Mottagarens kontosökning\nmisslyckades: " + r->errorString());
            } else if (selectedLanguage == "ENG") {
                ui->txtINFO->setText("Recipient's account retrieval\nfailed: " + r->errorString());
            } else {
                ui->txtINFO->setText("Vastaanottajan tilin haku\nepäonnistui: " + r->errorString());
            }

            r->deleteLater();
        }
    });
}

void TransferWindow::onReceiverDepositResponse(QNetworkReply *reply, double amount)
{
    ui->txtDebit->setVisible(false);
    ui->txtCredit->setVisible(false);
    mReceiverChoiceActive = false;

    if (reply->error() == QNetworkReply::NoError) {
        reply->deleteLater();

        logTransaction(senderAccountId, "withdrawal", amount, "ATM Transfer Out");
        logTransaction(receiverAccountId, "deposit", amount, "ATM Transfer In");

        ui->txtINFO->setText(translateMsg("Transfer successful") + QString(" (+%1 €)").arg(amount));
    }
    else {
        ui->txtINFO->setText(translateMsg("Transfer update failed: ") + reply->errorString());
        reply->deleteLater();
    }
}

void TransferWindow::logTransaction(int accountId, const QString &type, double amount, const QString &desc)
{
    QString url = Environment::base_url() + "/transaction";
    QNetworkRequest rq(url);
    rq.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    rq.setRawHeader("Authorization", mToken);

    QJsonObject o;
    o["idaccount"]   = accountId;
    o["type"]        = type;
    o["amount"]      = amount;
    o["description"] = desc;

    QJsonDocument d(o);
    QByteArray j = d.toJson();

    QNetworkReply *r = networkManager->post(rq, j);
    connect(r, &QNetworkReply::finished, this, [=]() {
        if (r->error() == QNetworkReply::NoError) {
            qDebug() << "Transaction logged successfully (" << type << ")";
        }
        else {
            qDebug() << "Failed to log transaction:" << r->errorString();
        }
        r->deleteLater();
    });
}

QString TransferWindow::translateMsg(const QString &englishMsg)
{
    if (selectedLanguage == "SWE") {
        if (englishMsg == "Insufficient funds") return "Otillräckliga medel";
        if (englishMsg == "Credit limit exceeded") return "Kreditgräns överskriden";
        if (englishMsg == "Withdrawal successful") return "Uttag lyckades";
        if (englishMsg.startsWith("Withdrawal update failed")) return "Uppdatering av uttaget misslyckades";
        if (englishMsg == "Transfer successful") return "Överföring lyckades";
        if (englishMsg.startsWith("Transfer update failed")) return "Uppdatering av överföringen misslyckades";
    }
    else if (selectedLanguage == "ENG") {
        return englishMsg;
    }
    else {
        if (englishMsg == "Insufficient funds") return "Tilillä ei ole katetta";
        if (englishMsg == "Credit limit exceeded") return "Luottoraja ylittyy";
        if (englishMsg == "Withdrawal successful") return "Nosto onnistui";
        if (englishMsg.startsWith("Withdrawal update failed")) return "Noston päivitys epäonnistui";
        if (englishMsg == "Transfer successful") return "Siirto onnistui";
        if (englishMsg.startsWith("Transfer update failed")) return "Siirron päivitys epäonnistui";
    }
    return englishMsg;
}

void TransferWindow::rollbackSenderWithdrawal(int accountId, double amount, const QString &desc)
{
    QString url = Environment::base_url() + "/account/" + QString::number(accountId);
    QNetworkRequest req(url);
    req.setRawHeader("Authorization", mToken);

    QNetworkReply *r = networkManager->get(req);
    connect(r, &QNetworkReply::finished, this, [=]() {
        if (r->error() == QNetworkReply::NoError) {
            QByteArray data = r->readAll();
            r->deleteLater();

            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonObject obj = doc.object();

            QString at = obj["type"].toString();
            if (at == "debit") {
                double cb = obj["debit_balance"].toString().toDouble();
                obj["debit_balance"] = QString::number(cb + amount, 'f', 2);
            }
            else {
                double cb = obj["credit_balance"].toString().toDouble();
                obj["credit_balance"] = QString::number(cb + amount, 'f', 2);
            }

            QJsonDocument upDoc(obj);
            QByteArray upData = upDoc.toJson();

            QNetworkRequest putReq(url);
            putReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            putReq.setRawHeader("Authorization", mToken);

            QNetworkReply *putReply = networkManager->sendCustomRequest(putReq, "PUT", upData);
            connect(putReply, &QNetworkReply::finished, this, [=]() {
                if (putReply->error() == QNetworkReply::NoError) {
                    putReply->deleteLater();
                    logTransaction(accountId, "rollback", amount, desc);
                }
                else {
                    qDebug() << "Failed to rollback money:" << putReply->errorString();
                    putReply->deleteLater();
                }
            });
        }
        else {
            qDebug() << "Failed to fetch sender account for rollback:" << r->errorString();
            r->deleteLater();
        }
    });
}

void TransferWindow::onDigitButtonClicked()
{
    QPushButton *b = qobject_cast<QPushButton*>(sender());
    if (!b) return;

    QString name = b->objectName();

    if (name == "btn_14") {
        if (ui->txtAccount->hasFocus()) {
            QString cur = ui->txtAccount->text();
            if (!cur.isEmpty()) {
                cur.chop(1);
                ui->txtAccount->setText(cur);
            }
        }
        else if (ui->txtAmount->hasFocus()) {
            QString cur = ui->txtAmount->text();
            if (!cur.isEmpty()) {
                cur.chop(1);
                ui->txtAmount->setText(cur);
            }
        }
        return;
    }

    QString value;

    if (name == "btn_10") {
        value = "+";
    }
    else if (name == "btn_11") {
        value = "-";
    }
    else if (name.startsWith("btn_")) {
        value = name.right(1);
    } else {
        return;
    }

    if (ui->txtAccount->hasFocus()) {
        ui->txtAccount->insert(value);
    }
    else if (ui->txtAmount->hasFocus()) {
        ui->txtAmount->insert(value);
    }
}

void TransferWindow::on_btnBack_clicked()
{
    this->hide();
    if (QWidget *p = qobject_cast<QWidget*>(parent())) {
        p->setGeometry(this->geometry());
        p->show();
    }
}

void TransferWindow::on_btnBack_2_clicked()
{
    this->hide();
    if (QWidget *p = qobject_cast<QWidget*>(parent())) {
        p->setGeometry(this->geometry());
        p->show();
    }
}
