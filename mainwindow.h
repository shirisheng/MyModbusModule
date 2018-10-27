#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "ModbusMaster.h"
#include "ModbusSlave.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    enum CommandSet
    {
        CommandSet_Read = 0x00,
        CommandSet_Write = 0x01,
    };

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setModbusMaster(ModbusMaster* pModbusMaster) { pModbusMaster_ = pModbusMaster;}
    void setModbusSlave(ModbusSlave* pModbusSlave) { pModbusSlave_ = pModbusSlave;}

public slots:
    void onSendCmd();
    void refreshPackBuffInfo();

private slots:
    void runningModbus();

private slots:
    void on_clearErrorButton_clicked();

    void on_startSendButton_clicked();

    void on_runningIntervalLineEdit_textChanged(const QString &arg1);

    void on_cmdSelectComboBox_currentIndexChanged(int index);

    void on_execCmdIntervalLineEdit_textChanged(const QString &arg1);

    void on_sendModeComboBox_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;
    bool isStartSend_;
    bool isSendContinue_;
    int currentCmd_;
    int continueSendAddr_;
    QTimer sendCmdTimer_;
    QTimer modbusRunningTimer_;
    QTimer packBuffInfoTimer_;
    ModbusMaster* pModbusMaster_;
    ModbusSlave* pModbusSlave_;
};

#endif // MAINWINDOW_H
