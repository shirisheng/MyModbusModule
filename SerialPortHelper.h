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
    enum ShowFormat
    {
        ShowFormat_Text = 0x00,
        ShowFormat_RowData = 0x01,
    };
    enum InputFormat
    {
        InputFormat_Text = 0x00,
        InputFormat_Hex = 0x01,
    };
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
    void sendAll(const QByteArray &data);
    qint64 readSerial(quint8* pBuff, quint16 len);
    qint64 writeSerial(quint8* pBuff, quint16 len);
    void showInCommBrowser(QString descr, QString content);
    void showInCommBrowser(QString descr, QByteArray content);
    QSerialPort * currentSerialPort(){return &currentSerialPort_;}

private slots:
    void onReadyRead();
    void onReadFinish();
    void onBytesWritten(qint64 bytes);
    void onSerialError(QSerialPort::SerialPortError error);

private slots:
    void on_refreshSerialButton_clicked();

    void on_saveSettingButton_clicked();

    void on_sendButton_clicked();

    void on_openSerialButton_clicked(bool checked);

    void on_serialSelectComboBox_currentIndexChanged(int index);

    void on_clearScreenButton_clicked();

    void on_stopScrollButton_clicked();

    void on_showFormatComboBox_currentIndexChanged(int index);

    void on_inputFormatComboBox_currentIndexChanged(int index);

private:
    Ui::SerialPortHelper *ui;
    static SerialPortHelper* pSerialPortHelper_;
    QList<QSerialPortInfo> availablePorts_;
    QSerialPort currentSerialPort_;
    int showFormat_;
    int inputFormat_;
    bool isScroll_;
    QTimer readFinishTimer_;
    QByteArray recvBuff_;
    QByteArray sendBuff_;
};

#endif // SERIALPORTHELPER_H
