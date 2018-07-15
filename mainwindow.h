#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "ModbusModule.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(ModbusModule* pModbusInstance, QWidget *parent = 0);
    ~MainWindow();

public slots:
    void onSendCmd();
    void refreshPackBuffInfo();

private slots:
    void runningModbus();
    void modbusTimerAdd();

private slots:
    void on_clearErrorButton_clicked();

    void on_startSendButton_clicked();

    void on_runningIntervalLineEdit_textChanged(const QString &arg1);

    void on_cmdSelectComboBox_currentIndexChanged(int index);

    void on_execCmdIntervalLineEdit_textChanged(const QString &arg1);

    void on_sendModeComboBox_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;
    bool cmdSelect_;
    bool sendMode_;
    bool startSendCmd_;
    int  continueSendAddr_;
    QTimer sendCmdTimer_;
    QTimer modbusTimer_;
    QTimer modbusRunningTimer_;
    QTimer packBuffInfoTimer_;
    ModbusModule* pModbusInstance_;
};

#endif // MAINWINDOW_H
