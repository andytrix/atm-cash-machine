#include "customerdata.h"
#include "mainmenu.h"
#include "environment.h"
#include "ui_mainmenu.h"
#include "withdraw.h"
#include "transfer.h"
#include "balance.h"
#include "transaction.h"
#include "login.h"

MainMenu::MainMenu(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainMenu)
{
    ui->setupUi(this);
    this->setWindowTitle("ATM");

    inactivityTimer = new QTimer(this);
    inactivityTimer->setInterval(30000); // 30 sec
    connect(inactivityTimer, &QTimer::timeout, this, &MainMenu::checkInactivity);

    qApp->installEventFilter(this);
}

MainMenu::~MainMenu()
{
    delete ui;
}

void MainMenu::showEvent(QShowEvent *event)
{
    inactivityTimer->start(30000);
    loadUserThumbnail(idcard);
    QDialog::showEvent(event);
}

void MainMenu::hideEvent(QHideEvent *event)
{
    inactivityTimer->stop();
    inactivityTimer->setInterval(30000);
    QDialog::hideEvent(event);
}

bool MainMenu::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress)
    {
        inactivityTimer->start(30000);
    }
    return QDialog::eventFilter(obj, event);
}

void MainMenu::checkInactivity()
{
    if (!this->isVisible())
    {
        inactivityTimer->stop();
        return;
    }

    this->hide();
    MainWindow *login = new MainWindow(this);
    login->setGeometry(this->geometry());
    login->show();
}

void MainMenu::setMyToken(const QByteArray &newMyToken)
{
    myToken = newMyToken;
    qDebug()<<"Main Menu";
    qDebug()<<myToken;
}

void MainMenu::setCardMode(const QString &mode)
{
    mCardMode = mode;  // Debit/Credit
}

void MainMenu::setUsername(const QString &newUsername)
{
    idcard = newUsername;
    loadUserThumbnail(idcard);

    QNetworkRequest request(Environment::base_url() + "/card/" + idcard);
    request.setRawHeader("Authorization", myToken);

    dataManager = new QNetworkAccessManager(this);
    connect(dataManager, &QNetworkAccessManager::finished, this, &MainMenu::onCardDataReceived);
    dataManager->get(request);
}

void MainMenu::setLanguage(const QString &newLanguage)
{
    selectedLanguage = newLanguage;
    qDebug() << "Language set in MainMenu: " << selectedLanguage;

    if (selectedLanguage == "FI") {
        ui->txtWithdraw->setText("Nosta");
        ui->txtBalance->setText("Saldo");
        ui->txtTransfer->setText("Siirrä");
        ui->txtTransaction->setText("Tapahtumat");
        ui->txtData->setText("Omat tiedot");
        ui->txtBack->setText("Kirjaudu ulos");
    }
    else if (selectedLanguage == "SWE") {
        ui->txtWithdraw->setText("Uttag");
        ui->txtBalance->setText("Saldo");
        ui->txtTransfer->setText("Överföring");
        ui->txtTransaction->setText("Transaktioner");
        ui->txtData->setText("Mina uppgifter");
        ui->txtBack->setText("Logga ut");
    }
    else if (selectedLanguage == "ENG") {
        ui->txtWithdraw->setText("Withdraw");
        ui->txtBalance->setText("Balance");
        ui->txtTransfer->setText("Transfer");
        ui->txtTransaction->setText("Transactions");
        ui->txtData->setText("My Data");
        ui->txtBack->setText("Log out");
    }
}

QString MainMenu::getLanguage() const
{
    return selectedLanguage;
}

void MainMenu::onCardDataReceived(QNetworkReply *reply)
{
    QByteArray response_data = reply->readAll();
    reply->deleteLater();
    dataManager->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        ui->labelUsername->setText("Error");
        return;
    }

    QJsonObject cardObj = QJsonDocument::fromJson(response_data).object();
    int customerId = cardObj["idcustomer"].toInt();
    if (customerId == 0) {
        ui->labelUsername->setText("Unknown");
        return;
    }

    QNetworkRequest custReq(Environment::base_url() + "/customer/" + QString::number(customerId));
    custReq.setRawHeader("Authorization", myToken);

    QNetworkAccessManager *customerDataManager = new QNetworkAccessManager(this);
    connect(customerDataManager, &QNetworkAccessManager::finished, this,
            [=](QNetworkReply *custReply) {
                QByteArray custData = custReply->readAll();
                custReply->deleteLater();
                customerDataManager->deleteLater();

                if (custReply->error() == QNetworkReply::NoError) {
                    QJsonObject custObj = QJsonDocument::fromJson(custData).object();

                    QString language = selectedLanguage;
                    QString welcomeText;

                    if (language == "FI") {
                        welcomeText = "Tervetuloa ";
                    } else if (language == "SWE") {
                        welcomeText = "Välkommen ";
                    } else if (language == "ENG") {
                        welcomeText = "Welcome ";
                    } else {
                        welcomeText = "Tervetuloa ";
                    }

                    ui->labelUsername->setText(welcomeText + custObj["fname"].toString()
                                               + " "
                                               + custObj["lname"].toString() + " !");
                } else {
                    ui->labelUsername->setText("Error");
                }
            });

    customerDataManager->get(custReq);
}

void MainMenu::loadUserThumbnail(const QString &cardId)
{
    QNetworkRequest request(Environment::base_url() + "/card/" + cardId);
    request.setRawHeader("Authorization", myToken);

    QNetworkAccessManager *cardDataManager = new QNetworkAccessManager(this);
    connect(cardDataManager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *cardReply) {
        QByteArray response_data = cardReply->readAll();
        cardReply->deleteLater();
        cardDataManager->deleteLater();

        if (cardReply->error() != QNetworkReply::NoError)
            return;

        QJsonObject cardObj = QJsonDocument::fromJson(response_data).object();
        int customerId = cardObj["idcustomer"].toInt();
        if (customerId == 0) {
            ui->lblThumbnail->clear();
            return;
        }

        QUrl url(Environment::base_url() + "/customer/getThumbnail?userId=" + QString::number(customerId));
        QNetworkRequest thumbRequest(url);
        thumbRequest.setRawHeader(QByteArray("Authorization"), myToken);

        QNetworkAccessManager *thumbnailManager = new QNetworkAccessManager(this);
        connect(thumbnailManager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *thumbReply) {
            if (thumbReply->error() == QNetworkReply::NoError) {
                QPixmap pixmap;
                if (pixmap.loadFromData(thumbReply->readAll()))
                    ui->lblThumbnail->setPixmap(pixmap.scaled(151, 151, Qt::KeepAspectRatio));
                else
                    ui->lblThumbnail->clear();
            }
            thumbReply->deleteLater();
            thumbnailManager->deleteLater();
        });

        thumbnailManager->get(thumbRequest);
    });

    cardDataManager->get(request);
}

void MainMenu::on_btnData_clicked()
{
    this->hide();
    CustomerData *objCustomerData = new CustomerData(this);
    objCustomerData->setGeometry(this->geometry());
    objCustomerData->setIdcard(idcard);
    objCustomerData->setMyToken(myToken);
    objCustomerData->setLanguage(selectedLanguage);
    objCustomerData->show();
    connect(objCustomerData, &CustomerData::finished, this, &MainMenu::show);
}

void MainMenu::on_btnWithdraw_clicked()
{
    this->hide();
    WithdrawWindow *objWithdrawWindow = new WithdrawWindow(this);
    objWithdrawWindow->setGeometry(this->geometry());
    objWithdrawWindow->setMyToken(myToken);
    objWithdrawWindow->setIdcard(idcard);
    objWithdrawWindow->setCardMode(mCardMode);
    objWithdrawWindow->setLanguage(selectedLanguage);
    objWithdrawWindow->show();
    connect(objWithdrawWindow, &WithdrawWindow::finished, this, &MainMenu::show);
}

void MainMenu::on_btnTransfer_clicked()
{
    this->hide();
    TransferWindow *objTransferWindow = new TransferWindow(this);
    objTransferWindow->setGeometry(this->geometry());
    objTransferWindow->setMyToken(myToken);
    objTransferWindow->setIdcard(idcard);
    objTransferWindow->setCardMode(mCardMode);
    objTransferWindow->setLanguage(selectedLanguage);
    objTransferWindow->show();
    connect(objTransferWindow, &TransferWindow::finished, this, &MainMenu::show);
}

void MainMenu::on_btnBalance_clicked()
{
    this->hide();
    BalanceWindow *objBalanceWindow = new BalanceWindow(this);
    objBalanceWindow->setGeometry(this->geometry());
    objBalanceWindow->setAuthToken(myToken);
    objBalanceWindow->setIdCard(idcard);
    objBalanceWindow->setLanguage(selectedLanguage);
    objBalanceWindow->show();
    connect(objBalanceWindow, &BalanceWindow::finished, this, &MainMenu::show);
}

void MainMenu::on_btnTransaction_clicked()
{
    this->hide();
    TransactionWindow *objTransactionWindow = new TransactionWindow(this);
    objTransactionWindow->setGeometry(this->geometry());
    objTransactionWindow->setLanguage(selectedLanguage);
    objTransactionWindow->fetchTransactions(idcard, mCardMode, myToken);
    objTransactionWindow->show();
    connect(objTransactionWindow, &TransactionWindow::finished, this, &MainMenu::show);
}

void MainMenu::on_btnLogout_clicked()
{
    this->hide();
    MainWindow *objMainWindow = new MainWindow(this);
    objMainWindow->setGeometry(this->geometry());
    objMainWindow->show();
}

void MainMenu::on_btnBack_2_clicked()
{
    this->hide();
    MainWindow *objMainWindow = new MainWindow(this);
    objMainWindow->setGeometry(this->geometry());
    objMainWindow->show();
}

void MainMenu::closeEvent(QCloseEvent *)
{
    QApplication::quit();
}

