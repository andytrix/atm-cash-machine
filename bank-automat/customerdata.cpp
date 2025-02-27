#include "customerdata.h"
#include "environment.h"
#include "ui_customerdata.h"
#include "mainmenu.h"
#include <QDebug>
#include <QFileDialog>
#include <QHttpMultiPart>
#include <QFile>
#include <QPixmap>
#include <QFileInfo>
#include <QApplication>

CustomerData::CustomerData(QWidget *parent) :
    QDialog(parent),
    currentPinState(Idle),
    ui(new Ui::CustomerData),
    customerId(0),
    thumbnailManager(new QNetworkAccessManager(this)),
    uploadManager(new QNetworkAccessManager(this)),
    pinChangeManager(nullptr)
{
    ui->setupUi(this);
    this->setWindowTitle("ATM");

    inactivityTimer = new QTimer(this);
    inactivityTimer->setInterval(10000); // 10 sec
    connect(inactivityTimer, &QTimer::timeout, this, &CustomerData::checkInactivity);
    qApp->installEventFilter(this);

    ui->txtChangePIN->setVisible(false);
    ui->labelPinINFO->setVisible(false);

    for (int i = 0; i < 12; ++i) {
        QString buttonName = "btn_" + QString::number(i);
        QPushButton *button = this->findChild<QPushButton *>(buttonName);
        if (button) {
            connect(button, &QPushButton::clicked, this, &CustomerData::onDigitButtonClicked);
        }
    }
    QPushButton *deleteButton = this->findChild<QPushButton *>("btn_14");
    if (deleteButton) {
        connect(deleteButton, &QPushButton::clicked, this, &CustomerData::onDigitButtonClicked);
    }

    connect(thumbnailManager, &QNetworkAccessManager::finished, this, &CustomerData::onThumbnailDownloaded);
    connect(uploadManager, &QNetworkAccessManager::finished, this, &CustomerData::onUploadThumbnailFinished);
    connect(ui->btnUploadThumbnail, &QPushButton::clicked, this, &CustomerData::onBtnUploadThumbnailClicked);
    connect(ui->txtChangePIN, &QLineEdit::textChanged, this, &CustomerData::on_txtChangePIN_textChanged);
}

CustomerData::~CustomerData()
{
    delete ui;
}

void CustomerData::setMyToken(const QByteArray &newMyToken) {
    myToken = newMyToken;
}

void CustomerData::setIdcard(const QString &newIdcard) {
    idcard = newIdcard;
}

void CustomerData::setLanguage(const QString &newLanguage) {
    selectedLanguage = newLanguage;
    updateLanguage();
}

void CustomerData::updateLanguage()
{
    if (selectedLanguage == "FI") {
        ui->labelData->setText("Asiakas ID:");
        ui->labelData_3->setText("Etunimi:");
        ui->labelData_2->setText("Sukunimi:");
        ui->txtBack->setText("Takaisin");
        ui->txtPicture->setText("Vaihda kuva");
        ui->txtPIN->setText("Vaihda PIN");
        ui->labelPinINFO->setText("Syötä vanha PIN");
    }
    else if (selectedLanguage == "SWE") {
        ui->labelData->setText("Kund ID:");
        ui->labelData_3->setText("Förnamn:");
        ui->labelData_2->setText("Efternamn:");
        ui->txtBack->setText("Tillbaka");
        ui->txtPicture->setText("Byt bild");
        ui->txtPIN->setText("Byt PIN");
        ui->labelPinINFO->setText("Ange den gamla PIN-koden");
    }
    else if (selectedLanguage == "ENG") {
        ui->labelData->setText("Customer ID:");
        ui->labelData_3->setText("First Name:");
        ui->labelData_2->setText("Last Name:");
        ui->txtBack->setText("Back");
        ui->txtPicture->setText("Change image");
        ui->txtPIN->setText("Change PIN");
        ui->labelPinINFO->setText("Enter the old PIN");
    }
}

void CustomerData::showEvent(QShowEvent *event)
{
    QString site_url = Environment::base_url() + "/card/" + idcard;
    QNetworkRequest request(site_url);

    //WEBTOKEN START
    request.setRawHeader(QByteArray("Authorization"), myToken);
    //WEBTOKEN END
    dataManager = new QNetworkAccessManager(this);

    connect(dataManager, &QNetworkAccessManager::finished, this, &CustomerData::showDataSlot);

    reply = dataManager->get(request);

    inactivityTimer->start(10000);
    QDialog::showEvent(event);
}

void CustomerData::hideEvent(QHideEvent *event)
{
    inactivityTimer->stop();
    inactivityTimer->setInterval(10000);
    QDialog::hideEvent(event);
}

bool CustomerData::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress)
    {
        inactivityTimer->start(10000);
    }
    return QDialog::eventFilter(obj, event);
}

void CustomerData::checkInactivity()
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

void CustomerData::showDataSlot(QNetworkReply *reply)
{
    response_data = reply->readAll();
    qDebug() << "Received data: " << response_data;

    QJsonDocument json_doc = QJsonDocument::fromJson(response_data);
    QJsonObject json_obj = json_doc.object();

    int id = json_obj["idcustomer"].toInt();
    customerId = id;
    ui->labelID->setText(QString::number(id));

    loadUserThumbnail(id);

    QString site_url = Environment::base_url() + "/customer/" + QString::number(id);
    QNetworkRequest request(site_url);
    request.setRawHeader(QByteArray("Authorization"), myToken);

    QNetworkAccessManager *customerDataManager = new QNetworkAccessManager(this);
    connect(customerDataManager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *customerReply) {
        QByteArray customerData = customerReply->readAll();
        qDebug() << "Customer information: " << customerData;

        QJsonDocument customerJsonDoc = QJsonDocument::fromJson(customerData);
        QJsonObject customerJsonObj = customerJsonDoc.object();

        ui->labelFname->setText(customerJsonObj["fname"].toString());
        ui->labelLname->setText(customerJsonObj["lname"].toString());

        customerReply->deleteLater();
        customerDataManager->deleteLater();
    });

    customerDataManager->get(request);

    reply->deleteLater();
    dataManager->deleteLater();
}

void CustomerData::loadUserThumbnail(int userId)
{
    QUrl url(Environment::base_url() + "/customer/getThumbnail?userId=" + QString::number(userId));
    QNetworkRequest request(url);
    request.setRawHeader(QByteArray("Authorization"), myToken);
    thumbnailManager->get(request);
}

void CustomerData::onThumbnailDownloaded(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray imageData = reply->readAll();
        QPixmap pixmap;
        if (pixmap.loadFromData(imageData)) {
            ui->lblThumbnail->setPixmap(pixmap.scaled(128, 128, Qt::KeepAspectRatio));
            qDebug() << "Thumbnail downloaded successfully!";
        } else {
            qDebug() << "Failed to load the image.";
        }
    } else {
        qDebug() << "Error downloading the thumbnail:" << reply->errorString();
    }
    reply->deleteLater();
}

void CustomerData::uploadNewThumbnail(int userId, QString filePath)
{
    QUrl apiUrl(Environment::base_url() + "/customer/thumbnail");
    QNetworkRequest request(apiUrl);
    request.setRawHeader(QByteArray("Authorization"), myToken);

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart userIdPart;
    userIdPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"userId\""));
    userIdPart.setBody(QString::number(userId).toUtf8());
    multiPart->append(userIdPart);

    QFile *file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open the file.";
        delete file;
        multiPart->deleteLater();
        return;
    }

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/jpeg"));
    QFileInfo fileInfo(file->fileName());
    QString fileName = fileInfo.fileName();
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"thumbnail\"; filename=\"" + fileName + "\""));
    imagePart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(imagePart);

    QNetworkReply *reply = uploadManager->post(request, multiPart);
    multiPart->setParent(reply);
}

void CustomerData::onUploadThumbnailFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "Thumbnail upload successful!";
        if (customerId != 0) {
            loadUserThumbnail(customerId);
        }
    } else {
        qDebug() << "Error uploading thumbnail:" << reply->errorString();
    }
    reply->deleteLater();
}

void CustomerData::onBtnUploadThumbnailClicked()
{
    ui->txtChangePIN->setVisible(false);
    ui->labelPinINFO->setVisible(false);
    ui->labelPinINFO->clear();
    ui->txtChangePIN->clear();

    inactivityTimer->stop();

    QString filePath = QFileDialog::getOpenFileName(this, "Select Image", "", "Images (*.png *.jpg *.jpeg)");

    inactivityTimer->start(10000);

    if (!filePath.isEmpty() && customerId != 0) {
        uploadNewThumbnail(customerId, filePath);
    }
}

void CustomerData::on_btnBack_clicked()
{
    this->hide();
    QWidget *parentWidget = qobject_cast<QWidget*>(parent());
    if (parentWidget) {
        parentWidget->setGeometry(this->geometry());
        parentWidget->show();
    }
}

void CustomerData::on_btnBack_2_clicked()
{
    this->hide();
    QWidget *parentWidget = qobject_cast<QWidget*>(parent());
    if (parentWidget) {
        parentWidget->setGeometry(this->geometry());
        parentWidget->show();
    }
}

void CustomerData::on_btnPIN_clicked() {
    ui->txtChangePIN->clear();
    ui->txtChangePIN->setVisible(true);
    ui->labelPinINFO->setVisible(true);
    currentPinState = EnterOld;
    ui->txtChangePIN->setFocus();
}

void CustomerData::onDigitButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button)
        return;

    QString buttonName = button->objectName();

    if (buttonName == "btn_14") {
        if (ui->txtChangePIN->hasFocus()) {
            QString currentText = ui->txtChangePIN->text();
            if (!currentText.isEmpty()) {
                currentText.chop(1);
                ui->txtChangePIN->setText(currentText);
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

    if (ui->txtChangePIN->hasFocus()) {
        ui->txtChangePIN->insert(value);
    }
}

void CustomerData::on_txtChangePIN_textChanged(const QString &text) {
    if (text.length() != 4) {
        return;
    }

    if (currentPinState == EnterOld) {
        verifiedOldPIN = text;
        verifyOldPIN(text);
        ui->txtChangePIN->clear();
    }
    else if (currentPinState == EnterNew) {
        candidateNewPIN = text;
        QTimer::singleShot(100, this, [this]() {
            currentPinState = ConfirmNew;

            if (selectedLanguage == "FI") {
                ui->labelPinINFO->setText("Syötä uusi PIN uudestaan");
            } else if (selectedLanguage == "SWE") {
                ui->labelPinINFO->setText("Ange den nya PIN-koden igen");
            } else if (selectedLanguage == "ENG") {
                ui->labelPinINFO->setText("Enter the new PIN again");
            }

            ui->txtChangePIN->clear();
        });
    }
    else if (currentPinState == ConfirmNew) {
        if (text == candidateNewPIN) {
            changePIN(verifiedOldPIN, candidateNewPIN);
        } else {
            if (selectedLanguage == "FI") {
                ui->labelPinINFO->setText("PIN ei täsmää, syötä uusi PIN uudestaan");
            } else if (selectedLanguage == "SWE") {
                ui->labelPinINFO->setText("PIN-koden matchar inte, ange den nya PIN-koden igen");
            } else if (selectedLanguage == "ENG") {
                ui->labelPinINFO->setText("PIN does not match, enter the new PIN again");
            }

            currentPinState = EnterNew;
        }
        ui->txtChangePIN->clear();
    }
}

void CustomerData::verifyOldPIN(const QString &oldPin) {
    QJsonObject jsonObj;
    jsonObj.insert("idcard", idcard);
    jsonObj.insert("pin", oldPin);

    QString site_url = Environment::base_url() + "/login";
    QNetworkRequest request(site_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished,
            this, &CustomerData::onVerifyOldPinFinished);
    manager->post(request, QJsonDocument(jsonObj).toJson());
}

void CustomerData::onVerifyOldPinFinished(QNetworkReply *reply) {
    QByteArray response = reply->readAll();
    qDebug() << "VerifyOldPIN response:" << response;

    if (response.length() < 2) {
        if (selectedLanguage == "FI") {
            ui->labelPinINFO->setText("Palvelin ei vastaa!");
        } else if (selectedLanguage == "SWE") {
            ui->labelPinINFO->setText("Servern svarar inte!");
        } else if (selectedLanguage == "ENG") {
            ui->labelPinINFO->setText("Server is not responding!");
        }
        currentPinState = EnterOld;
    }
    else if (response == "!404!") {
        if (selectedLanguage == "FI") {
            ui->labelPinINFO->setText("Tietokantavirhe!");
        } else if (selectedLanguage == "SWE") {
            ui->labelPinINFO->setText("Databasfel!");
        } else if (selectedLanguage == "ENG") {
            ui->labelPinINFO->setText("Database error!");
        }
        currentPinState = EnterOld;
    }
    else if (response == "false" || response.length() <= 20) {
        if (selectedLanguage == "FI") {
            ui->labelPinINFO->setText("Väärä PIN, yritä uudelleen");
        } else if (selectedLanguage == "SWE") {
            ui->labelPinINFO->setText("Fel PIN, försök igen");
        } else if (selectedLanguage == "ENG") {
            ui->labelPinINFO->setText("Incorrect PIN, try again");
        }
        currentPinState = EnterOld;
    }
    else {
        if (selectedLanguage == "FI") {
            ui->labelPinINFO->setText("Syötä uusi PIN");
        } else if (selectedLanguage == "SWE") {
            ui->labelPinINFO->setText("Ange den nya PIN-koden");
        } else if (selectedLanguage == "ENG") {
            ui->labelPinINFO->setText("Enter the new PIN");
        }
        currentPinState = EnterNew;
    }
    reply->deleteLater();
}

void CustomerData::changePIN(const QString &oldPin, const QString &newPin) {
    QUrl apiUrl(Environment::base_url() + "/card/" + idcard);
    QNetworkRequest request(apiUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", myToken);

    QJsonObject json;
    json.insert("idcustomer", customerId);
    json.insert("pin", newPin);
    json.insert("cardtype", "single");
    json.insert("status", "active");
    json.insert("expiry_date", "2099-12-31");

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished,
    this, &CustomerData::onChangePINFinished);
    QNetworkReply* reply = manager->sendCustomRequest(request, "PUT", data);
}

void CustomerData::onChangePINFinished(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray response = reply->readAll();
        qDebug() << "PIN change response:" << response;
        QString responseStr = QString::fromUtf8(response);
        if (responseStr.contains("Card updated", Qt::CaseInsensitive)) {
            if (selectedLanguage == "FI") {
                ui->labelPinINFO->setText("PIN vaihdettu");
            } else if (selectedLanguage == "SWE") {
                ui->labelPinINFO->setText("PIN-kod uppdaterad");
            } else if (selectedLanguage == "ENG") {
                ui->labelPinINFO->setText("PIN changed");
            }
            ui->labelPinINFO->setVisible(false);
            ui->txtChangePIN->setVisible(false);
        } else {
            if (selectedLanguage == "FI") {
                ui->labelPinINFO->setText("PIN:n vaihto epäonnistui");
            } else if (selectedLanguage == "SWE") {
                ui->labelPinINFO->setText("Misslyckades med att byta PIN-kod");
            } else if (selectedLanguage == "ENG") {
                ui->labelPinINFO->setText("PIN change failed");
            }
        }
    } else {
        qDebug() << "Error changing PIN:" << reply->errorString();
        if (selectedLanguage == "FI") {
            ui->labelPinINFO->setText("Virhe PIN:n vaihdossa");
        } else if (selectedLanguage == "SWE") {
            ui->labelPinINFO->setText("Fel vid byte av PIN-kod");
        } else if (selectedLanguage == "ENG") {
            ui->labelPinINFO->setText("Error changing PIN");
        }
    }
    reply->deleteLater();
    QNetworkAccessManager *manager = qobject_cast<QNetworkAccessManager*>(sender());
    if (manager) {
        manager->deleteLater();
    }
    currentPinState = Idle;
}

void CustomerData::closeEvent(QCloseEvent *)
{
    QApplication::quit();
}
