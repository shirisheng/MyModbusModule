#include "mainwindow.h"
#include <QApplication>
#include "ModbusModule.h"
#include "SerialPortHelper.h"

SerialPortHelper* pSerialPort1;
SerialPortHelper* pSerialPort2;
/****************************此部分的函数由使用者自己实现****************************start*/
Sint16 serial1Write(Uint8* pPackBuff, Uint16 packLen)
{
    return pSerialPort1->sendData(pPackBuff,packLen);
}

Sint16 serial1Read(Uint8* pRecvBuff, Uint16 recvLen)
{
    return pSerialPort1->currentSerialPort()->read((char*)pRecvBuff,recvLen);
}

void dataHandler1(Uint16* pUnPackData, Uint16 dataLen, Uint16 descr)
{
    pSerialPort1->showInCommBrowser(QString("回调 : void dataHandler(Uint16* pUnPackData, Uint16 dataLen)"),QString(""));
}

void errorHandler1(void)
{
    pSerialPort1->showInCommBrowser(QString("回调 : void commErrorHandler(void)"),QString(""));
}

Sint16 serial2Write(Uint8* pPackBuff, Uint16 packLen)
{
    return pSerialPort2->sendData(pPackBuff,packLen);
}

Sint16 serial2Read(Uint8* pRecvBuff, Uint16 recvLen)
{
    return pSerialPort2->currentSerialPort()->read((char*)pRecvBuff,recvLen);
}

void dataHandler2(Uint16* pUnPackData, Uint16 dataLen, Uint16 descr)
{
    pSerialPort2->showInCommBrowser(QString("回调 : void dataHandler(Uint16* pUnPackData, Uint16 dataLen)"),QString(""));
}

void errorHandler2(void)
{
    pSerialPort2->showInCommBrowser(QString("回调 : void commErrorHandler(void)"),QString(""));
}
/****************************此部分的函数由由使用者自己实现******************************end*/
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    pSerialPort1 = new SerialPortHelper;
    pSerialPort2 = new SerialPortHelper;
    ModbusInitStruct initStruct;
    initStruct.timeOutTime = 100;
    initStruct.reSendTimes = 10;
    initStruct.recvBuffLen = 200;
    initStruct.packBuffLen = 100;
    initStruct.callBack.pWriteData = serial1Write;
    initStruct.callBack.pReadData = serial1Read;
    initStruct.callBack.pDataHandler = dataHandler1;
    initStruct.callBack.pErrorHandler = errorHandler1;
    ModbusModule modbusInstance1(initStruct);
    modbusInstance1.setSerialPort(pSerialPort1);

    initStruct.timeOutTime = 100;
    initStruct.reSendTimes = 10;
    initStruct.recvBuffLen = 200;
    initStruct.packBuffLen = 200;
    initStruct.callBack.pWriteData = serial2Write;
    initStruct.callBack.pReadData = serial2Read;
    initStruct.callBack.pDataHandler = dataHandler2;
    initStruct.callBack.pErrorHandler = errorHandler2;
    ModbusModule modbusInstance2(initStruct);
    modbusInstance2.setSerialPort(pSerialPort2);

    MainWindow w1(&modbusInstance1);
    MainWindow w2(&modbusInstance2);
    w1.setWindowTitle("Modbus Debuger1");
    w2.setWindowTitle("Modbus Debuger3");
    w1.show();
    w2.show();
    pSerialPort1->setWindowTitle("Serial Port1");
    pSerialPort2->setWindowTitle("Serial Port3");
    pSerialPort1->show();
    pSerialPort2->show();
    return a.exec();
}
