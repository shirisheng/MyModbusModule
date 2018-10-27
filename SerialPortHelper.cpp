#include <QDebug>
#include <QStringList>
#include "SerialPortHelper.h"
#include "ui_SerialPortHelper.h"

SerialPortHelper* SerialPortHelper::pSerialPortHelper_ = NULL;
SerialPortHelper::SerialPortHelper(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::SerialPortHelper),
    showFormat_(0),
    inputFormat_(0),
    isScroll_(true)
{
    ui->setupUi(this);
    this->setWindowTitle("Serial Port Helper");
    QObject::connect(&currentSerialPort_, SIGNAL(error(QSerialPort::SerialPortError)),
                     this, SLOT(onSerialError(QSerialPort::SerialPortError)));
}

SerialPortHelper::~SerialPortHelper()
{
    delete ui;
}

void SerialPortHelper::showInCommBrowser(QString descr, QString content)
{
    if(!isScroll_) return;
    descr.append("\n");
    ui->commStatusBrowser->insertPlainText(descr + content);
    ui->commStatusBrowser->insertPlainText(QString("\n\n\n"));
    ui->commStatusBrowser->moveCursor(QTextCursor::End);
}

void SerialPortHelper::showInCommBrowser(QString descr, QByteArray content)
{
    if(!isScroll_) return;
    QString contentStr(descr);
    contentStr.append("\n");
    if(showFormat_ == ShowFormat_RowData)
    {
        for(int i = 0; i < content.size(); i++)
        {
            quint8 tmpVal = (quint8)content.at(i);
            contentStr.append(tmpVal < 0x10 ? QString("0") : QString(""));
            contentStr.append(QString::number(tmpVal, 16)).append(" ");
        }
    }
    else if(showFormat_ == ShowFormat_Text)
        contentStr.append(content.data());
    ui->commStatusBrowser->insertPlainText(contentStr);
    ui->commStatusBrowser->insertPlainText(QString("\n\n\n"));
    ui->commStatusBrowser->moveCursor(QTextCursor::End);
}

qint64 SerialPortHelper::readSerial(quint8* pBuff, quint16 len)
{
    if(!currentSerialPort_.isOpen()
    || currentSerialPort_.error() != QSerialPort::NoError)
        return -1;
    QObject::disconnect(&currentSerialPort_, SIGNAL(readyRead()),
                        this, SLOT(onReadyRead()));
    return currentSerialPort_.read((char*)pBuff, len);
}

qint64 SerialPortHelper::writeSerial(quint8* pBuff, quint16 len)
{
    if(!currentSerialPort_.isOpen()
    || currentSerialPort_.error() != QSerialPort::NoError)
        return -1;
    QObject::disconnect(&currentSerialPort_, SIGNAL(bytesWritten(qint64)),
                     this, SLOT(onBytesWritten(qint64)));
    return currentSerialPort_.write((char*)pBuff, len);
}

void SerialPortHelper::sendAll(const QByteArray &data)
{
    if(!currentSerialPort_.isOpen()
    || currentSerialPort_.error() != QSerialPort::NoError)
        return;
    this->recvBuff_.clear();
    this->sendBuff_.clear();
    QObject::connect(&currentSerialPort_, SIGNAL(bytesWritten(qint64)),
                     this, SLOT(onBytesWritten(qint64)));
    QObject::connect(&currentSerialPort_, SIGNAL(readyRead()),
                     this, SLOT(onReadyRead()));
    QObject::connect(&readFinishTimer_, SIGNAL(timeout()),
                     this, SLOT(onReadFinish()));
    this->sendBuff_.append(data);
    currentSerialPort_.write(sendBuff_);
    showInCommBrowser(QString("Send Content:"),sendBuff_);
}

void SerialPortHelper::onReadyRead()
{
    if(!currentSerialPort_.isOpen()
    || currentSerialPort_.error() != QSerialPort::NoError)
        return;
    readFinishTimer_.start(5);
    QByteArray recvData = currentSerialPort_.readAll();
    recvBuff_.append(recvData);
}

void SerialPortHelper::onReadFinish()
{
    readFinishTimer_.stop();
    QObject::disconnect(&currentSerialPort_, SIGNAL(readyRead()),
                        this, SLOT(onReadyRead()));
    QObject::disconnect(&readFinishTimer_, SIGNAL(timeout()),
                        this, SLOT(onReadFinish()));
    QString descr("recv content:");
    showInCommBrowser(descr,recvBuff_);
}

void SerialPortHelper::onBytesWritten(qint64 bytes)
{
    this->sendBuff_.remove(0,bytes);
    if(sendBuff_.isEmpty())
    {
        QObject::disconnect(&currentSerialPort_, SIGNAL(bytesWritten(qint64)),
                            this, SLOT(onBytesWritten(qint64)));
        return;
    }
    currentSerialPort_.write(sendBuff_);
}

void SerialPortHelper::onSerialError(QSerialPort::SerialPortError error)
{
    if(!currentSerialPort_.isOpen())
        return;
    currentSerialPort_.clear();
    currentSerialPort_.clearError();
    showInCommBrowser(QString("Serial Error : "),QString::number(error));
}

void SerialPortHelper::on_refreshSerialButton_clicked()
{
    QString content;
    ui->serialSelectComboBox->clear();
    ui->serialSelectComboBox->clearEditText();
    availablePorts_ = QSerialPortInfo::availablePorts();
    if(availablePorts_.isEmpty()) return;
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        content.append("Serial Port : ").append(info.portName()).append("\n");
        content.append("Description : ").append(info.description()).append("\n");
        content.append("Manufacturer: ").append(info.manufacturer()).append("\n\n");
        if (ui->serialSelectComboBox->currentText() != info.portName())
        {
            ui->serialSelectComboBox->addItem(info.portName());
        }
    }
    showInCommBrowser(QString("Available Serial Port:"),content);
}

void SerialPortHelper::on_saveSettingButton_clicked()
{
    QString content;
    if(!currentSerialPort_.isOpen()) return;
    currentSerialPort_.setBaudRate(ui->baudRateSetComboBox->currentText().toInt());
    currentSerialPort_.setDataBits((QSerialPort::DataBits)ui->dataBitsSetComboBox->currentText().toInt());
    if(ui->paritySetComboBox->currentIndex() > 0)
        currentSerialPort_.setParity((QSerialPort::Parity)(ui->paritySetComboBox->currentIndex() + 1));
    else currentSerialPort_.setParity((QSerialPort::Parity)ui->paritySetComboBox->currentIndex());
    currentSerialPort_.setStopBits((QSerialPort::StopBits)ui->stopBitsSetComboBox->currentText().toInt());
    currentSerialPort_.setFlowControl((QSerialPort::FlowControl)ui->flowCtrlSetComboBox->currentIndex());
    content.append("PortName    : ").append(ui->serialSelectComboBox->currentText()).append("\n");
    content.append("BaudRate    : ").append(ui->baudRateSetComboBox->currentText()).append("\n");
    content.append("DataBits    : ").append(ui->dataBitsSetComboBox->currentText()).append("\n");
    content.append("Parity      : ").append(ui->paritySetComboBox->currentText()).append("\n");
    content.append("stopBits    : ").append(ui->stopBitsSetComboBox->currentText()).append("\n");
    content.append("FlowControl : ").append(ui->flowCtrlSetComboBox->currentText()).append("\n");
    showInCommBrowser(QString("Serial Port Setting:"),content);
}

void SerialPortHelper::on_sendButton_clicked()
{
    QString contentToSend;
    contentToSend = ui->sendContentEdit->toPlainText();
    if(inputFormat_ == InputFormat_Hex)
    {
        QByteArray dataTosend;
        contentToSend.remove(QRegExp("^\\s+"));
        contentToSend.remove(QRegExp("\\s+$"));
        QStringList strList = contentToSend.split(QRegExp("\\s+"));
        for(int i = 0; i < strList.size(); i++)
        {
            bool convertOk = false;
            int tempVal = strList[i].toInt(&convertOk,16);
            if(strList.at(i).size() > 2 || !convertOk)
            {
                QString error("Hex format input error!");
                showInCommBrowser(error,QString(""));
                return;
            }
            dataTosend.append(tempVal);
        }
        this->sendAll(dataTosend);
    }
    else if(inputFormat_ == InputFormat_Text)
    {
        this->sendAll(contentToSend.toUtf8());
    }
}

void SerialPortHelper::on_openSerialButton_clicked(bool checked)
{
    if(checked)
    {
        QString portName = ui->serialSelectComboBox->currentText();
        currentSerialPort_.setPortName(portName); /// 选取串口
        if(!currentSerialPort_.open(QIODevice::ReadWrite))
        { /// 打开串口
            showInCommBrowser(QString(""),QString(portName).
                              append(" Open failure!"));
            ui->openSerialButton->setChecked(false);
            ui->openSerialButton->setText(tr("Open Port"));
        }
        else
        {
            showInCommBrowser(QString(""),QString(portName).
                              append(" Open successfully!"));
            ui->openSerialButton->setText(tr("Close Port"));
        }
    }
    else
    {
        currentSerialPort_.close();
        ui->openSerialButton->setText(tr("Open Port"));
        showInCommBrowser(QString(""),QString(currentSerialPort_.portName()).
                          append(" Close successfully!"));
    }
}

void SerialPortHelper::on_serialSelectComboBox_currentIndexChanged(int index)
{
    if(currentSerialPort_.isOpen())
        currentSerialPort_.close();
    ui->openSerialButton->setChecked(false);
    ui->openSerialButton->setText(tr("Open Port"));
}

void SerialPortHelper::on_clearScreenButton_clicked()
{
    ui->commStatusBrowser->clear();
}

void SerialPortHelper::on_stopScrollButton_clicked()
{
    isScroll_ = !isScroll_;
    if(isScroll_)
        ui->stopScrollButton->setText(tr("Stop Scroll"));
    else ui->stopScrollButton->setText(tr("Scroll"));
}

void SerialPortHelper::on_showFormatComboBox_currentIndexChanged(int index)
{
    this->showFormat_ = index;
}

void SerialPortHelper::on_inputFormatComboBox_currentIndexChanged(int index)
{
    this->inputFormat_ = index;
}
