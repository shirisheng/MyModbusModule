#include "mainwindow.h"
#include <QApplication>
#include "ModbusModule.h"
#include "SerialPortHelper.h"

SerialPortHelper* pSerialPort1;
SerialPortHelper* pSerialPort2;
/****************************此部分的函数由使用者自己实现****************************start*/
Int16 serialPortSender1(Uint8* pPackBuff, Uint16 packLen)
{
    return pSerialPort1->sendData(pPackBuff,packLen);
}

Int16 serialPortReceiver1(Uint8* pRecvBuff, Uint16 recvLen)
{
    return pSerialPort1->currentSerialPort()->read((char*)pRecvBuff,recvLen);
}

void dataHandler1(Uint16* pUnPackData, Uint16 dataLen)
{
    pSerialPort1->showInCommBrowser(QString("回调 : void dataHandler(Uint16* pUnPackData, Uint16 dataLen)"),QString(""));
}

void commErrorHandler1(void)
{
    pSerialPort1->showInCommBrowser(QString("回调 : void commErrorHandler(void)"),QString(""));
}

Int16 serialPortSender2(Uint8* pPackBuff, Uint16 packLen)
{
    return pSerialPort2->sendData(pPackBuff,packLen);
}

Int16 serialPortReceiver2(Uint8* pRecvBuff, Uint16 recvLen)
{
    return pSerialPort2->currentSerialPort()->read((char*)pRecvBuff,recvLen);
}

void dataHandler2(Uint16* pUnPackData, Uint16 dataLen)
{
    pSerialPort2->showInCommBrowser(QString("回调 : void dataHandler(Uint16* pUnPackData, Uint16 dataLen)"),QString(""));
}

void commErrorHandler2(void)
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
    initStruct.timeOutTime = 50;
    initStruct.reSendTimes = 10;
    initStruct.recvBuffLen = 200;
    initStruct.packBuffLen = 100;
    initStruct.dataBuffLen = 100;
    initStruct.callBack.pDataSender = serialPortSender1;
    initStruct.callBack.pDataReceiver = serialPortReceiver1;
    initStruct.callBack.pDataHandler = dataHandler1;
    initStruct.callBack.pCommErrorHandler = commErrorHandler1;
    ModbusModule modbusInstance1(initStruct);
    modbusInstance1.setSerialPort(pSerialPort1);

    initStruct.timeOutTime = 50;
    initStruct.reSendTimes = 10;
    initStruct.recvBuffLen = 200;
    initStruct.packBuffLen = 200;
    initStruct.dataBuffLen = 200;
    initStruct.callBack.pDataSender = serialPortSender2;
    initStruct.callBack.pDataReceiver = serialPortReceiver2;
    initStruct.callBack.pDataHandler = dataHandler2;
    initStruct.callBack.pCommErrorHandler = commErrorHandler2;
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
