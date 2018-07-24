#include <QDebug>
#include "SerialPortHelper.h"
#include "ui_SerialPortHelper.h"

SerialPortHelper* SerialPortHelper::pSerialPortHelper_ = NULL;
SerialPortHelper::SerialPortHelper(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::SerialPortHelper),
    showMode_(true),
    stopScroll_(false)
{
    ui->setupUi(this);
    this->setWindowTitle("Serial Port Helper");
//    QObject::connect(&timer_, SIGNAL(timeout()), this, SLOT(recvFinish()));
//    QObject::connect(&currentSerialPort_, SIGNAL(readyRead()), this, SLOT(recvData()));
    QObject::connect(&currentSerialPort_, SIGNAL(bytesWritten(qint64)), this, SLOT(sendContinue(qint64)));
}

SerialPortHelper::~SerialPortHelper()
{
    delete ui;
}

void SerialPortHelper::showInCommBrowser(QString descr, QString strToShow)
{
    if(stopScroll_) return;
    descr.append("\n");
    ui->commStatusBrowser->insertPlainText(descr + strToShow);
    ui->commStatusBrowser->insertPlainText(QString("\n\n\n"));
    ui->commStatusBrowser->moveCursor(QTextCursor::End);
}

void SerialPortHelper::showInCommBrowser(QString descr, QByteArray dataToShow)
{
    if(stopScroll_) return;
    QString strToShow(descr);
    strToShow.append("\n");
    if(!showMode_)
    {
        for(int i = 0; i < dataToShow.size(); i++)
            strToShow.append(QString::number((quint8)dataToShow.at(i))).append(" ");
    }
    else strToShow.append(dataToShow.data());
    ui->commStatusBrowser->insertPlainText(strToShow);
    ui->commStatusBrowser->insertPlainText(QString("\n\n\n"));
    ui->commStatusBrowser->moveCursor(QTextCursor::End);
}

//void SerialPortHelper::recvData()
//{
//    timer_.start(5);
//    QByteArray recvData = currentSerialPort_.readAll();
//    recvBuff_.append(recvData);
//}

//void SerialPortHelper::recvFinish()
//{
//    timer_.stop();
//    QString descr("接收内容:");
//    showInCommBrowser(descr,recvBuff_);
//    emit recvFinishSignal((quint8 *)recvBuff_.data(), recvBuff_.size());
//    recvBuff_.clear();
//}

qint64 SerialPortHelper::sendData(const QByteArray &data)
{
    this->sendBuff_.append(data);
    qint64 bytes = currentSerialPort_.write(sendBuff_);
    this->sendBuff_.remove(0,bytes);
    showInCommBrowser(QString("发送内容:"),data);
    return bytes;
}

qint64 SerialPortHelper::sendData(quint8* pPackBuff, quint16 packLen)
{
    return sendData(QByteArray((char*)pPackBuff, packLen));
}

void SerialPortHelper::sendContinue(qint64 writtenBytes)
{
    if(sendBuff_.isEmpty()) return;
    qint64 bytes = currentSerialPort_.write(sendBuff_);
    this->sendBuff_.remove(0,bytes);
}

void SerialPortHelper::on_refreshSerialButton_clicked()
{
    QString strToShow;
    ui->serialSelectComboBox->clear();
    ui->serialSelectComboBox->clearEditText();
    availablePorts_ = QSerialPortInfo::availablePorts();
    if(availablePorts_.isEmpty()) return;
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        strToShow.append("Serial Port : ").append(info.portName()).append("\n");
        strToShow.append("Description : ").append(info.description()).append("\n");
        strToShow.append("Manufacturer: ").append(info.manufacturer()).append("\n\n");
        if (ui->serialSelectComboBox->currentText() != info.portName())
        {
            ui->serialSelectComboBox->addItem(info.portName());
        }
    }
    showInCommBrowser(QString("Available Serial Port:"),strToShow);
}

void SerialPortHelper::on_saveSettingButton_clicked()
{
    QString strToShow;
    if(!currentSerialPort_.isOpen()) return;
    currentSerialPort_.setBaudRate(ui->baudRateSetComboBox->currentText().toInt());
    currentSerialPort_.setDataBits((QSerialPort::DataBits)ui->dataBitsSetComboBox->currentText().toInt());
    if(ui->paritySetComboBox->currentIndex() > 0)
        currentSerialPort_.setParity((QSerialPort::Parity)(ui->paritySetComboBox->currentIndex() + 1));
    else currentSerialPort_.setParity((QSerialPort::Parity)ui->paritySetComboBox->currentIndex());
    currentSerialPort_.setStopBits((QSerialPort::StopBits)ui->stopBitsSetComboBox->currentText().toInt());
    currentSerialPort_.setFlowControl((QSerialPort::FlowControl)ui->flowCtrlSetComboBox->currentIndex());
    strToShow.append("PortName    : ").append(ui->serialSelectComboBox->currentText()).append("\n");
    strToShow.append("BaudRate    : ").append(ui->baudRateSetComboBox->currentText()).append("\n");
    strToShow.append("DataBits    : ").append(ui->dataBitsSetComboBox->currentText()).append("\n");
    strToShow.append("Parity      : ").append(ui->paritySetComboBox->currentText()).append("\n");
    strToShow.append("stopBits    : ").append(ui->stopBitsSetComboBox->currentText()).append("\n");
    strToShow.append("FlowControl : ").append(ui->flowCtrlSetComboBox->currentText()).append("\n");
    showInCommBrowser(QString("Serial Port Setting:"),strToShow);
}

void SerialPortHelper::on_sendButton_clicked()
{
    QString contentToSend = ui->sendContentEdit->toPlainText();
    currentSerialPort_.write(contentToSend.toUtf8().data(),contentToSend.toUtf8().size());
    showInCommBrowser(QString("Send Content:"),contentToSend.toUtf8());
//    timer_.singleShot(5,this, SLOT(readData()));
}

void SerialPortHelper::on_openSerialButton_clicked(bool checked)
{
    if(checked)
    {
        QString portName = ui->serialSelectComboBox->currentText();
        currentSerialPort_.setPortName(portName); //选取串口
        if(!currentSerialPort_.open(QIODevice::ReadWrite))
        { //打开串口
            showInCommBrowser(QString(""),QString(portName).
                              append(" open failure!"));
            ui->openSerialButton->setChecked(false);
            ui->openSerialButton->setText(tr("Open Port"));
        }
        else
        {
            showInCommBrowser(QString(""),QString(portName).
                              append(" open successfully!"));
            ui->openSerialButton->setText(tr("Close Port"));
        }
    }
    else
    {
        currentSerialPort_.close();
        ui->openSerialButton->setText(tr("Open Port"));
        showInCommBrowser(QString(""),QString(currentSerialPort_.portName()).
                          append(" open successfully!"));
    }
}

void SerialPortHelper::on_serialSelectComboBox_currentIndexChanged(int index)
{
    currentSerialPort_.close();
    ui->openSerialButton->setChecked(false);
    ui->openSerialButton->setText(tr("Open Port"));
}

void SerialPortHelper::on_clearScreenButton_clicked()
{
    ui->commStatusBrowser->clear();
}

void SerialPortHelper::on_showModeButton_clicked()
{
    showMode_ = !showMode_;
    if(showMode_)
        ui->showModeButton->setText(tr("Text"));
    else ui->showModeButton->setText(tr("Row Data"));
}

void SerialPortHelper::on_stopScrollButton_clicked()
{
    stopScroll_ = !stopScroll_;
    if(stopScroll_)
        ui->stopScrollButton->setText(tr("Scroll"));
    else ui->stopScrollButton->setText(tr("Stop Scroll"));
}
