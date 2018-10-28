#include "mainwindow.h"
#include "ui_mainwindow.h"

extern Uint16 modbusTimer1;
extern Uint16 modbusTimer2;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    isStartSend_ = false;
    isSendContinue_ = false;
    currentCmd_ = 0;
    continueSendAddr_ = 0;
    pModbusMaster_ = NULL;
    pModbusSlave_ = NULL;
    this->setWindowTitle("Modbus Debuger");
    ui->endAddrLineEdit->setEnabled(isSendContinue_);
    QObject::connect(&sendCmdTimer_,
                     SIGNAL(timeout()), this, SLOT(onSendCmd()));
    QObject::connect(&modbusRunningTimer_,
                     SIGNAL(timeout()), this, SLOT(runningModbus()));
    QObject::connect(&packBuffInfoTimer_,
                     SIGNAL(timeout()), this, SLOT(refreshPackBuffInfo()));
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
    isStartSend_ = !isStartSend_;
    if(isStartSend_)
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
    if(isStartSend_)
    {
        bool convertOk1 = false;
        bool convertOk2 = false;
        bool convertOk3 = false;
        bool convertOk4 = false;
        if(currentCmd_ == CommandSet_Read)
        {
            Uint8 slaveID = ui->readSlaveIDLineEdit->text().toInt(&convertOk1,10);
            Uint16 editAddr = ui->readStartAddrLineEdit->text().toInt(&convertOk2,10);
            Uint16 dataNum = ui->readDataNumLineEdit->text().toInt(&convertOk3,10);
            Uint16 endAddr = ui->endAddrLineEdit->text().toInt(&convertOk4,10);
            Uint16 startAddr = isSendContinue_ ? continueSendAddr_ : editAddr;
            if(convertOk1 && convertOk2 && convertOk3 && convertOk4)
            {
                pModbusMaster_->readDataFromSlave(slaveID, startAddr, dataNum);
                continueSendAddr_ < endAddr ? (continueSendAddr_ += dataNum)  : continueSendAddr_ = editAddr;
            }
        }
        else if(currentCmd_ == CommandSet_Write)
        {
            Uint8 slaveID = ui->writeSlaveIDLineEdit->text().toInt(&convertOk1,10);
            Uint16 editAddr = ui->writeAddrLineEdit->text().toInt(&convertOk2,10);
            Uint16 value = ui->writeValueLineEdit->text().toInt(&convertOk3,10);
            Uint16 endAddr = ui->endAddrLineEdit->text().toInt(&convertOk4,10);
            Uint16 currAddr = isSendContinue_ ? continueSendAddr_ : editAddr;
            if(convertOk1 && convertOk2 && convertOk3 && convertOk4)
            {
                pModbusMaster_->writeDataToSlave(slaveID, currAddr, value);
                continueSendAddr_ < endAddr ? continueSendAddr_++ : continueSendAddr_ = editAddr;
            }
        }
        if(!isSendContinue_) ui->startSendButton->click();
    }
}

void MainWindow::on_runningIntervalLineEdit_textChanged(const QString &arg1)
{
    modbusRunningTimer_.stop();
    modbusRunningTimer_.start(arg1.toInt());
}

void MainWindow::on_cmdSelectComboBox_currentIndexChanged(int index)
{
    currentCmd_ = index;
}

void MainWindow::on_execCmdIntervalLineEdit_textChanged(const QString &arg1)
{
    sendCmdTimer_.stop();
    sendCmdTimer_.start(arg1.toInt());
}

void MainWindow::on_sendModeComboBox_currentIndexChanged(int index)
{
    isSendContinue_ = !isSendContinue_;
    if(currentCmd_ == CommandSet_Read)
        continueSendAddr_ = ui->readStartAddrLineEdit->text().toInt();
    else if(currentCmd_ == CommandSet_Write)
        continueSendAddr_ = ui->writeAddrLineEdit->text().toInt();
    ui->endAddrLineEdit->setEnabled(isSendContinue_);
}
