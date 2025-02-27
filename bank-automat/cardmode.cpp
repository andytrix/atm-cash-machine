#include "cardmode.h"
#include "login.h"
#include "ui_cardmode.h"
#include "mainmenu.h"
#include <QApplication>

CardMode::CardMode(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CardMode)
{
    ui->setupUi(this);
    this->setWindowTitle("ATM");

    inactivityTimer = new QTimer(this);
    inactivityTimer->setInterval(10000);
    connect(inactivityTimer, &QTimer::timeout, this, &CardMode::checkInactivity);

    qApp->installEventFilter(this);
}

CardMode::~CardMode()
{
    delete ui;
}

void CardMode::showEvent(QShowEvent *event)
{
    inactivityTimer->start(10000);
    QDialog::showEvent(event);
}

void CardMode::hideEvent(QHideEvent *event)
{
    inactivityTimer->stop();
    inactivityTimer->setInterval(10000);
    QDialog::hideEvent(event);
}

bool CardMode::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress)
    {
        inactivityTimer->start(10000);
    }
    return QDialog::eventFilter(obj, event);
}

void CardMode::checkInactivity()
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

void CardMode::setUsername(const QString &username)
{
    mUsername = username;
}

void CardMode::setMyToken(const QString &token)
{
    mToken = token;
}

void CardMode::setLanguage(const QString &lang)
{
    mLanguage = lang;

    if (mLanguage == "ENG") {
        ui->txtDebit->setText("Debit");
        ui->txtCredit->setText("Credit");
    }
    else if (mLanguage == "SWE") {
        ui->txtDebit->setText("Debet");
        ui->txtCredit->setText("Kredit");
    }
    else {
        ui->txtDebit->setText("Debit");
        ui->txtCredit->setText("Credit");
    }
}

void CardMode::on_btnBack_clicked()
{
    this->hide();
    MainWindow *login = new MainWindow(this);
    login->setGeometry(this->geometry());
    login->show();
}

void CardMode::on_btnDebit_clicked()
{
    this->hide();
    MainMenu *objMainMenu = new MainMenu(this);
    objMainMenu->setGeometry(this->geometry());
    objMainMenu->setLanguage(mLanguage);
    objMainMenu->setMyToken(mToken.toUtf8());
    objMainMenu->setUsername(mUsername);
    objMainMenu->setCardMode("debit");
    objMainMenu->show();
    connect(objMainMenu, &MainMenu::finished, this, &CardMode::show);
}

void CardMode::on_btnCredit_clicked()
{
    this->hide();
    MainMenu *objMainMenu = new MainMenu(this);
    objMainMenu->setGeometry(this->geometry());
    objMainMenu->setLanguage(mLanguage);
    objMainMenu->setMyToken(mToken.toUtf8());
    objMainMenu->setUsername(mUsername);
    objMainMenu->setCardMode("credit");
    objMainMenu->show();
    connect(objMainMenu, &MainMenu::finished, this, &CardMode::show);
}

void CardMode::closeEvent(QCloseEvent *)
{
    QApplication::quit();
}
