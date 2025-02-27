#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDialog>
#include <QCloseEvent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnStart_clicked();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
