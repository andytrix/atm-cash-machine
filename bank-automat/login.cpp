#include "environment.h"
#include "login.h"
#include "mainmenu.h"
#include "cardmode.h"
#include "ui_login.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QCloseEvent>
#include <QDebug>

Login::Login(MainWindow *mainWin, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Login)
    , mainWindow(mainWin)
{
    ui->setupUi(this);
    this->setWindowTitle("ATM");

    for (int i = 0; i < 12; ++i) {
        QString buttonName = "btn_" + QString::number(i);
        QPushButton *button = this->findChild<QPushButton *>(buttonName);
        if (button) {
            connect(button, &QPushButton::clicked, this, &Login::onDigitButtonClicked);
        }
    }

    QPushButton *deleteButton = this->findChild<QPushButton *>("btn_14");
    if (deleteButton) {
        connect(deleteButton, &QPushButton::clicked, this, &Login::onDigitButtonClicked);
    }

    inactivityTimer = new QTimer(this);
    connect(inactivityTimer, &QTimer::timeout, this, &Login::checkInactivity);

    qApp->installEventFilter(this);
}

Login::~Login()
{
    delete ui;
}

void Login::showEvent(QShowEvent *event)
{
    inactivityTimer->start(10000);
    QDialog::showEvent(event);
}

void Login::hideEvent(QHideEvent *event)
{
    inactivityTimer->stop();
    inactivityTimer->setInterval(10000);
    QDialog::hideEvent(event);
}

bool Login::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress)
    {
        inactivityTimer->start(10000);
    }
    return QDialog::eventFilter(obj, event);
}

void Login::checkInactivity()
{
    if (!this->isVisible())
    {
        inactivityTimer->stop();
        return;
    }

    this->hide();
    if (mainWindow)
    {
        MainWindow *mainWindow = new MainWindow(this);
        mainWindow->setGeometry(this->geometry());
        mainWindow->show();
    }
}

void Login::on_btnLogin_clicked()
{
    if (!timerLocked) {
        QJsonObject jsonObj;
        jsonObj.insert("idcard", ui->numberIdcard->text());
        jsonObj.insert("pin", ui->numberPin->text());

        QString site_url = Environment::base_url() + "/login";
        QNetworkRequest request(site_url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        postManager = new QNetworkAccessManager(this);
        connect(postManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(LoginSlot(QNetworkReply*)));

        reply = postManager->post(request, QJsonDocument(jsonObj).toJson());
    }
}

void Login::LoginSlot(QNetworkReply *reply)
{
    response_data = reply->readAll();
    qDebug() << "LoginSlot response_data:" << response_data;

    if (response_data.length() < 2) {
        qDebug() << "Server is not responding!";
        if (selectedLanguage == "FI") {
            ui->labelInfo->setText("Palvelin ei vastaa!");
        } else if (selectedLanguage == "SWE") {
            ui->labelInfo->setText("Servern svarar inte!");
        } else {
            ui->labelInfo->setText("Server is not responding!");
        }
    } else if (response_data == "!404!") {
        if (selectedLanguage == "FI") {
            ui->labelInfo->setText("Tietokantavirhe!");
        } else if (selectedLanguage == "SWE") {
            ui->labelInfo->setText("Databasfel!");
        } else {
            ui->labelInfo->setText("Database error!");
        }
    } else {
        if (response_data == "locked") {
            timerLocked = true;
            if (selectedLanguage == "FI") {
                ui->labelInfo->setText("Kortti on lukittu!");
            } else if (selectedLanguage == "SWE") {
                ui->labelInfo->setText("Kortet är låst!");
            } else {
                ui->labelInfo->setText("Card is locked!");
            }
            inactivityTimer->stop();
            inactivityTimer->setInterval(10000);
            QTimer::singleShot(3000, this, [this]() {
                this->hide();
                MainWindow *objMainWindow = new MainWindow(this);
                objMainWindow->setGeometry(this->geometry());
                objMainWindow->show();
                timerLocked = false;
            });
            return;
        } else if (response_data != "false" && response_data.length() > 20) {
            if (selectedLanguage == "FI") {
                ui->labelInfo->setText("");
            } else if (selectedLanguage == "SWE") {
                ui->labelInfo->setText("");
            } else {
                ui->labelInfo->setText("");
            }

            myToken = "Bearer " + response_data;

            QString card_url = Environment::base_url() + "/card/" + ui->numberIdcard->text();
            QNetworkRequest request(card_url);
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            request.setRawHeader("Authorization", myToken.toUtf8());

            getCardManager = new QNetworkAccessManager(this);
            connect(getCardManager, SIGNAL(finished(QNetworkReply*)),
                    this, SLOT(cardSlot(QNetworkReply*)));
            getCardManager->get(request);
        } else {
            if (selectedLanguage == "FI") {
                ui->labelInfo->setText("Väärä ID/PIN!");
            } else if (selectedLanguage == "SWE") {
                ui->labelInfo->setText("Fel ID/PIN!");
            } else {
                ui->labelInfo->setText("Incorrect ID/PIN!");
            }
        }
    }

    reply->deleteLater();
    postManager->deleteLater();
}

void Login::cardSlot(QNetworkReply *reply)
{
    QByteArray cardResponse = reply->readAll();
    qDebug() << "cardSlot response_data:" << cardResponse;

    QJsonDocument doc = QJsonDocument::fromJson(cardResponse);
    if (!doc.isObject()) {
        if (selectedLanguage == "FI") {
            ui->labelInfo->setText("Virheellinen JSON (kortti) -vastaus!");
        } else if (selectedLanguage == "SWE") {
            ui->labelInfo->setText("Ogiltigt JSON-svar (kort)!");
        } else {
            ui->labelInfo->setText("Invalid JSON (card) response!");
        }
        qDebug() << "Card response is not a JSON object";
    } else {
        QJsonObject json = doc.object();
        QString cardType = json.value("cardtype").toString();
        qDebug() << "Card type:" << cardType;

        if (cardType == "dual") {
            CardMode *objCardMode = new CardMode(this);
            objCardMode->setGeometry(this->geometry());
            objCardMode->setUsername(ui->numberIdcard->text());
            objCardMode->setMyToken(myToken);
            objCardMode->setLanguage(selectedLanguage);
            objCardMode->show();
            connect(objCardMode, &CardMode::finished, this, &Login::show);
            this->hide();
        } else if (cardType == "single") {
            QString accountUrl = Environment::base_url() + "/card_account/" + ui->numberIdcard->text();
            QNetworkRequest accountRequest(accountUrl);
            accountRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            accountRequest.setRawHeader("Authorization", myToken.toUtf8());
            QNetworkAccessManager *accountManager = new QNetworkAccessManager(this);
            connect(accountManager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *accountReply) {
                QByteArray accountResponse = accountReply->readAll();
                QJsonDocument accountDoc = QJsonDocument::fromJson(accountResponse);
                QString accountType = "debit";
                if (accountDoc.isObject()) {
                    accountType = accountDoc.object().value("type").toString();
                    if (!(accountType == "credit" || accountType == "debit"))
                        accountType = "debit";
                }
                MainMenu *objMainMenu = new MainMenu(this);
                objMainMenu->setGeometry(this->geometry());
                objMainMenu->setMyToken(myToken.toUtf8());
                objMainMenu->setUsername(ui->numberIdcard->text());
                objMainMenu->setLanguage(selectedLanguage);
                objMainMenu->setCardMode(accountType);
                objMainMenu->show();
                connect(objMainMenu, &MainMenu::finished, this, &Login::show);
                this->hide();
                accountReply->deleteLater();
                accountManager->deleteLater();
            });
            accountManager->get(accountRequest);
        }
    }

    reply->deleteLater();
    getCardManager->deleteLater();
}

void Login::on_btnLangFI_clicked()
{
    selectedLanguage = "FI";
    qDebug() << "Language set: Suomi";
    updateLanguage();
}

void Login::on_btnLangSWE_clicked()
{
    selectedLanguage = "SWE";
    qDebug() << "Language set: Svenska";
    updateLanguage();
}

void Login::on_btnLangENG_clicked()
{
    selectedLanguage = "ENG";
    qDebug() << "Language set: English";
    updateLanguage();
}

void Login::updateLanguage()
{
    if (selectedLanguage == "ENG") {
        ui->label->setText("Card ID:");
        ui->label_2->setText("Card PIN:");
        ui->btnLogin->setText("OK");
    }
    else if (selectedLanguage == "SWE") {
        ui->label->setText("Kortnummer:");
        ui->label_2->setText("Kort-PIN:");
        ui->btnLogin->setText("OK");
    }
    else {
        ui->label->setText("Kortin tunnus:");
        ui->label_2->setText("Kortin PIN:");
        ui->btnLogin->setText("OK");
    }
}

void Login::onDigitButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button)
        return;

    QString buttonName = button->objectName();

    if (buttonName == "btn_14") {
        if (ui->numberIdcard->hasFocus()) {
            QString currentText = ui->numberIdcard->text();
            if (!currentText.isEmpty()) {
                currentText.chop(1);
                ui->numberIdcard->setText(currentText);
            }
        }
        else if (ui->numberPin->hasFocus()) {
            QString currentText = ui->numberPin->text();
            if (!currentText.isEmpty()) {
                currentText.chop(1);
                ui->numberPin->setText(currentText);
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

    if (ui->numberIdcard->hasFocus()) {
        ui->numberIdcard->insert(value);
    }
    else if (ui->numberPin->hasFocus()) {
        ui->numberPin->insert(value);
    }
}

void Login::on_btn_Stop_clicked()
{
    this->hide();
    MainWindow *objMainWindow = new MainWindow(this);
    objMainWindow->setGeometry(this->geometry());
    objMainWindow->show();
}

void Login::closeEvent(QCloseEvent *)
{
    QApplication::quit();
}
