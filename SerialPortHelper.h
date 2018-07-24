#ifndef SERIALPORTHELPER_H
#define SERIALPORTHELPER_H

#include <QFrame>
#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>

typedef quint8 Uint8;
typedef quint16 Uint16;

namespace Ui {
class SerialPortHelper;
}

class SerialPortHelper : public QFrame
{
    Q_OBJECT

public:
    explicit SerialPortHelper(QWidget *parent = 0);
    ~SerialPortHelper();
    static SerialPortHelper* pSerialPortHelper()
    {
        if(pSerialPortHelper_ == NULL)
        {
            pSerialPortHelper_ = new(SerialPortHelper);
        }
        return pSerialPortHelper_;
    }
    void showInCommBrowser(QString descr, QString strToShow);
    void showInCommBrowser(QString descr, QByteArray dataToShow);
    QSerialPort * currentSerialPort(){return &currentSerialPort_;}

signals:
    recvFinishSignal(Uint8* pRecvBuff, Uint16 buffLen);

public slots:
//    void recvData();
    void sendData(const QByteArray &data);
    void sendData(quint8* pPackBuff, quint16 packLen);

private slots:
//    void recvFinish();
    void sendContinue(qint64 writtenBytes);

private slots:
    void on_refreshSerialButton_clicked();

    void on_saveSettingButton_clicked();

    void on_sendButton_clicked();

    void on_openSerialButton_clicked(bool checked);

    void on_serialSelectComboBox_currentIndexChanged(int index);

    void on_clearScreenButton_clicked();

    void on_showModeButton_clicked();

    void on_stopScrollButton_clicked();

private:
    Ui::SerialPortHelper *ui;
    QTimer timer_;
    bool showMode_;
    bool stopScroll_;
    QByteArray recvBuff_;
    QByteArray sendBuff_;
    QList<QSerialPortInfo> availablePorts_;
    QSerialPort currentSerialPort_;
    static SerialPortHelper* pSerialPortHelper_;
};

#endif // SERIALPORTHELPER_H
