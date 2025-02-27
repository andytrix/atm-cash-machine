#include "login.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("ATM");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnStart_clicked()
{
    this->hide();

    Login *objLogin = new Login(this);
    objLogin->setGeometry(this->geometry());
    objLogin->show();

    connect(objLogin, &Login::finished, this, &MainWindow::show);
}

void MainWindow::closeEvent(QCloseEvent *)
{
    QApplication::quit();
}
