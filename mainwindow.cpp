#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ModbusModule.h"

extern Uint16 modbusTimer1;
extern Uint16 modbusTimer2;
MainWindow::MainWindow(ModbusModule* pModbusInstance, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    cmdSelect_ = false;
    sendMode_ = false;
    startSendCmd_ = false;
    continueSendAddr_ = 0;
    pModbusInstance_ = pModbusInstance;
    this->setWindowTitle("Modbus Debuger");
    QObject::connect(&sendCmdTimer_, SIGNAL(timeout()), this, SLOT(onSendCmd()));
    QObject::connect(&modbusTimer_, SIGNAL(timeout()), this, SLOT(modbusTimerAdd()));
    QObject::connect(&modbusRunningTimer_, SIGNAL(timeout()), this, SLOT(runningModbus()));
    QObject::connect(&packBuffInfoTimer_, SIGNAL(timeout()), this, SLOT(refreshPackBuffInfo()));
    modbusTimer_.start(1);
    modbusRunningTimer_.start(0);
    packBuffInfoTimer_.start(5);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::runningModbus()
{
    pModbusInstance_->modbusRunningController();
}

void MainWindow::modbusTimerAdd()
{
    modbusTimer1++;
    modbusTimer2++;
}

void MainWindow::refreshPackBuffInfo()
{
    ui->buffSizeLineEdit->setText(QString::number(pModbusInstance_->packBuffSize()));
    ui->buffFrontLineEdit->setText(QString::number(pModbusInstance_->packBuffFront()));
    ui->buffRearineEdit->setText(QString::number(pModbusInstance_->packBuffRear()));
}

void MainWindow::on_clearErrorButton_clicked()
{
    pModbusInstance_->clearErrAndSendNextPack();
}

void MainWindow::on_startSendButton_clicked()
{
    startSendCmd_ = !startSendCmd_;
    if(startSendCmd_)
    {
        ui->startSendButton->setText("Stop Send");
        sendCmdTimer_.start(ui->execCmdIntervalLineEdit->text().toInt());
    }
    else
    {
        ui->startSendButton->setText("Start Send");
        sendCmdTimer_.stop();
    }
}

void MainWindow::onSendCmd()
{
    if(startSendCmd_)
    {
        if(!cmdSelect_)
        {
            pModbusInstance_->readDataFromSlave(ui->readSlaveIDLineEdit->text().toInt(),
                              !sendMode_ ? ui->readStartAddrLineEdit->text().toInt() : continueSendAddr_,
                              ui->readDataNumLineEdit->text().toInt());
            continueSendAddr_ < 200 ? continueSendAddr_++ : continueSendAddr_ = ui->readStartAddrLineEdit->text().toInt();
        }
        else
        {
            pModbusInstance_->writeDataToSlave(ui->writeSlaveIDLineEdit->text().toInt(),
                             !sendMode_ ? ui->writeAddrLineEdit->text().toInt() : continueSendAddr_,
                             !sendMode_ ? ui->writeValueLineEdit->text().toInt() : continueSendAddr_ + ui->writeValueLineEdit->text().toInt());
            continueSendAddr_ < 200 ? continueSendAddr_++ : continueSendAddr_ = ui->writeAddrLineEdit->text().toInt();
        }
        if(!sendMode_) ui->startSendButton->click();
    }
}

void MainWindow::on_runningIntervalLineEdit_textChanged(const QString &arg1)
{
    modbusRunningTimer_.stop();
    modbusRunningTimer_.start(arg1.toInt());
}

void MainWindow::on_cmdSelectComboBox_currentIndexChanged(int index)
{
    cmdSelect_ = !cmdSelect_;
}

void MainWindow::on_execCmdIntervalLineEdit_textChanged(const QString &arg1)
{
    sendCmdTimer_.stop();
    sendCmdTimer_.start(arg1.toInt());
}

void MainWindow::on_sendModeComboBox_currentIndexChanged(int index)
{
    sendMode_ = !sendMode_;
    if(!cmdSelect_)
    continueSendAddr_ = ui->readStartAddrLineEdit->text().toInt();
    else continueSendAddr_ = ui->writeAddrLineEdit->text().toInt();
}
