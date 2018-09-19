#include "mainwindow.h"
#include "ui_mainwindow.h"

extern Uint16 modbusTimer1;
extern Uint16 modbusTimer2;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    cmdSelect_ = false;
    sendMode_ = false;
    startSendCmd_ = false;
    continueSendAddr_ = 0;
    pModbusMaster_ = NULL;
    pModbusSlave_ = NULL;
    this->setWindowTitle("Modbus Debuger");
    QObject::connect(&sendCmdTimer_, SIGNAL(timeout()), this, SLOT(onSendCmd()));
    QObject::connect(&modbusRunningTimer_, SIGNAL(timeout()), this, SLOT(runningModbus()));
    QObject::connect(&packBuffInfoTimer_, SIGNAL(timeout()), this, SLOT(refreshPackBuffInfo()));
    modbusRunningTimer_.start(0);
    packBuffInfoTimer_.start(5);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::runningModbus()
{
    if(pModbusMaster_ != NULL)
    {
        pModbusMaster_->runModbusMaster();
    }
    if(pModbusSlave_ != NULL)
    {
        pModbusSlave_->runModbusSlave();
    }
}

void MainWindow::refreshPackBuffInfo()
{
#ifdef DEBUG_CODE
    ui->buffSizeLineEdit->setText(QString::number(pModbusMaster_->sendBuffSize()));
    ui->buffFrontLineEdit->setText(QString::number(pModbusMaster_->sendBuffFront()));
    ui->buffRearineEdit->setText(QString::number(pModbusMaster_->sendBuffRear()));
#endif
}

void MainWindow::on_clearErrorButton_clicked()
{
    pModbusMaster_->clearMasterError();
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
    bool convertOk;
    if(startSendCmd_)
    {
        if(!cmdSelect_)
        {
            pModbusMaster_->readDataFromSlave(ui->readSlaveIDLineEdit->text().toInt(),
                              !sendMode_ ? ui->readStartAddrLineEdit->text().toInt(&convertOk,16) : continueSendAddr_,
                              ui->readDataNumLineEdit->text().toInt());
            continueSendAddr_ < ui->endAddrLineEdit->text().toInt(&convertOk,16) ?
                        continueSendAddr_++ : continueSendAddr_ = ui->readStartAddrLineEdit->text().toInt(&convertOk,16);
        }
        else
        {
            pModbusMaster_->writeDataToSlave(ui->writeSlaveIDLineEdit->text().toInt(),
                             !sendMode_ ? ui->writeAddrLineEdit->text().toInt(&convertOk,16) : continueSendAddr_,
                             ui->writeValueLineEdit->text().toInt());
            continueSendAddr_ < ui->endAddrLineEdit->text().toInt(&convertOk,16) ?
                        continueSendAddr_++ : continueSendAddr_ = ui->writeAddrLineEdit->text().toInt(&convertOk,16);
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
