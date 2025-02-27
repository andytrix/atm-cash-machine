#include "environment.h"
#include "transaction.h"
#include "ui_transaction.h"

TransactionWindow::TransactionWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransactionWindow),
    currentPage(0)
{
    ui->setupUi(this);
    this->setWindowTitle("ATM");

    inactivityTimer = new QTimer(this);
    inactivityTimer->setInterval(10000); // 10 sec
    connect(inactivityTimer, &QTimer::timeout, this, &TransactionWindow::checkInactivity);
    qApp->installEventFilter(this);

    connect(ui->btnNext, &QPushButton::clicked, this, &TransactionWindow::on_btnNext_clicked);
    connect(ui->btnPrev, &QPushButton::clicked, this, &TransactionWindow::on_btnPrev_clicked);

    ui->tableWidget->setColumnCount(5);
    QStringList headers = {"ID", "Type", "Description", "Amount", "Timestamp"};
    ui->tableWidget->setHorizontalHeaderLabels(headers);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

    ui->tableWidget->setColumnHidden(0, true);
    ui->tableWidget->setColumnHidden(2, true);
    ui->tableWidget->verticalHeader()->setFixedWidth(2);
}

TransactionWindow::~TransactionWindow()
{
    delete ui;
}

void TransactionWindow::showEvent(QShowEvent *event)
{
    inactivityTimer->start(10000);
    QDialog::showEvent(event);
}

void TransactionWindow::hideEvent(QHideEvent *event)
{
    inactivityTimer->stop();
    inactivityTimer->setInterval(10000);
    QDialog::hideEvent(event);
}

bool TransactionWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress)
    {
        inactivityTimer->start(10000);
    }
    return QDialog::eventFilter(obj, event);
}

void TransactionWindow::checkInactivity()
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

void TransactionWindow::setLanguage(const QString &newLanguage)
{
    selectedLanguage = newLanguage;
    updateLanguage();
}

void TransactionWindow::updateLanguage()
{
    if (selectedLanguage == "FI") {
        ui->txtBack->setText("Takaisin");
        ui->txtNext->setText("Seuraava");
        ui->txtPrevious->setText("Edellinen");

        QStringList headers = {"ID", "TYYPPI", "KUVAUS", "SUMMA", "AIKALEIMA"};
        ui->tableWidget->setHorizontalHeaderLabels(headers);
    }
    else if (selectedLanguage == "SWE") {
        ui->txtBack->setText("Tillbaka");
        ui->txtNext->setText("Nästa");
        ui->txtPrevious->setText("Föregående");

        QStringList headers = {"ID", "TYP", "BESKRIVNING", "BELOPP", "TIDSSTÄMPEL"};
        ui->tableWidget->setHorizontalHeaderLabels(headers);
    }
    else if (selectedLanguage == "ENG") {
        ui->txtBack->setText("Back");
        ui->txtNext->setText("Next");
        ui->txtPrevious->setText("Previous");

        QStringList headers = {"ID", "TYPE", "DESCRIPTION", "AMOUNT", "TIMESTAMP"};
        ui->tableWidget->setHorizontalHeaderLabels(headers);
    }
}

void TransactionWindow::fetchTransactions(const QString &idcard, const QString &cardMode, const QByteArray &token)
{
    QString url = Environment::base_url() + "/card_account?idcard=" + idcard;
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", token);

    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *reply) {
        onCardAccountReceived(reply, idcard, cardMode, token);
    });
    networkManager->get(request);
}

void TransactionWindow::onCardAccountReceived(QNetworkReply *reply, const QString &idcard, const QString &cardMode, const QByteArray &token)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Error fetching card account:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    reply->deleteLater();

    qDebug() << "Raw card account data (JSON):" << responseData;

    QJsonArray arr = QJsonDocument::fromJson(responseData).array();
    if (arr.isEmpty()) {
        qDebug() << "No account found for this card ID!";
        return;
    }

    int selectedAccountId = -1;

    for (const QJsonValue &val : arr) {
        QJsonObject obj = val.toObject();
        qDebug() << "Checking account:" << obj;

        if (obj["type"].toString().trimmed().toLower() == cardMode.trimmed().toLower() &&
            QString::number(obj["idcard"].toInt()) == idcard.trimmed()) {

            selectedAccountId = obj["idaccount"].toInt();
            qDebug() << "Matching account found! ID:" << selectedAccountId;
            break;
        }
    }

    if (selectedAccountId == -1) {
        qDebug() << "No matching account found for mode:" << cardMode;
        return;
    }

    fetchTransactionsByAccount(selectedAccountId, token);
}

void TransactionWindow::fetchTransactionsByAccount(int accountId, const QByteArray &token)
{
    QString url = Environment::base_url() + "/transaction/account/" + QString::number(accountId);
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", token);

    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &TransactionWindow::onTransactionDataReceived);
    networkManager->get(request);
}

void TransactionWindow::onTransactionDataReceived(QNetworkReply *reply)
{
    QByteArray responseData = reply->readAll();
    reply->deleteLater();
    networkManager->deleteLater();

    if (reply->error() == QNetworkReply::NoError)
    {
        qDebug() << "Raw Transaction Data (JSON):" << responseData;

        transactionsList.clear();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

        if (!jsonDoc.isArray()) {
            qDebug() << "Error: JSON response is not an array!";
            return;
        }

        QJsonArray transactions = jsonDoc.array();
        qDebug() << "Parsed JSON transactions count:" << transactions.size();

        for (const QJsonValue &value : transactions)
        {
            QJsonObject obj = value.toObject();
            qDebug() << "Transaction:" << obj;
            transactionsList.append(obj);
        }

        currentPage = 0;
        updateTransactionList();
    }
    else
    {
        qDebug() << "Error fetching transactions:" << reply->errorString();
    }
}

void TransactionWindow::updateTransactionList()
{
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    int startIndex = currentPage * itemsPerPage;
    int endIndex = qMin(startIndex + itemsPerPage, transactionsList.size());

    for (int i = startIndex; i < endIndex; ++i) {
        QJsonObject transaction = transactionsList[i];

        ui->tableWidget->insertRow(i - startIndex);

        QFont cellFont;
        cellFont.setPointSize(11);

        ui->tableWidget->setItem(i - startIndex, 0, new QTableWidgetItem(QString::number(transaction["idtransaction"].toInt())));
        ui->tableWidget->item(i - startIndex, 0)->setFont(cellFont);

        ui->tableWidget->setItem(i - startIndex, 1, new QTableWidgetItem(transaction["type"].toString()));
        ui->tableWidget->item(i - startIndex, 1)->setFont(cellFont);

        ui->tableWidget->setItem(i - startIndex, 2, new QTableWidgetItem(transaction["description"].toString()));
        ui->tableWidget->item(i - startIndex, 2)->setFont(cellFont);

        ui->tableWidget->setItem(i - startIndex, 3, new QTableWidgetItem(transaction["amount"].toString()));
        ui->tableWidget->item(i - startIndex, 3)->setFont(cellFont);

        QString timestamp = transaction["timestamp"].toString();
        QDateTime dateTime = QDateTime::fromString(timestamp, Qt::ISODate);
        if (dateTime.isValid()) {
            timestamp = dateTime.toString("yyyy-MM-dd hh:mm:ss");
        } else {
            timestamp = "Invalid Date";
        }

        ui->tableWidget->setItem(i - startIndex, 4, new QTableWidgetItem(timestamp));
        ui->tableWidget->item(i - startIndex, 4)->setFont(cellFont);
    }

    ui->tableWidget->resizeColumnsToContents();

    ui->btnPrev->setEnabled(currentPage > 0);
    ui->btnNext->setEnabled(endIndex < transactionsList.size());
    ui->btnNext->setEnabled(true);
    ui->btnPrev->setEnabled(true);
}

void TransactionWindow::on_btnNext_clicked()
{
    if ((currentPage + 1) * itemsPerPage < transactionsList.size()) {
        currentPage++;
        updateTransactionList();
    }
}

void TransactionWindow::on_btnPrev_clicked()
{
    if (currentPage > 0) {
        currentPage--;
        updateTransactionList();
    }
}

void TransactionWindow::on_btnBack_clicked()
{
    this->hide();
    QWidget *parentWidget = qobject_cast<QWidget*>(parent());
    if (parentWidget) {
        parentWidget->setGeometry(this->geometry());
        parentWidget->show();
    }
}

void TransactionWindow::on_btnBack_2_clicked()
{
    this->hide();
    QWidget *parentWidget = qobject_cast<QWidget*>(parent());
    if (parentWidget) {
        parentWidget->setGeometry(this->geometry());
        parentWidget->show();
    }
}

void TransactionWindow::closeEvent(QCloseEvent *)
{
    QApplication::quit();
}
