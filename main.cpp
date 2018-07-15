#include "mainwindow.h"
#include <QApplication>
#include "ModbusModule.h"
#include "SerialPortHelper.h"

#define RECV_BUFF1_LEN      100
#define RECV_BUFF2_LEN      200

SerialPortHelper* pSerialPort1;
SerialPortHelper* pSerialPort2;
Uint16 modbusTimer1;
Uint16 modbusTimer2;
Uint8 recvBuff1[RECV_BUFF1_LEN];
Uint8 recvBuff2[RECV_BUFF2_LEN];
/****************************此部分的函数由使用者自己实现****************************start*/
void serialPortSender1(Uint8* pPackBuff, Uint16 packLen)
{
    pSerialPort1->sendData(pPackBuff,packLen);
}

void dataHandler1(Uint16* pUnPackData, Uint16 dataLen)
{
    pSerialPort1->showInCommBrowser(QString("回调 : void dataHandler(Uint16* pUnPackData, Uint16 dataLen)"),QString(""));
}

void commErrorHandler1(void)
{
    pSerialPort1->showInCommBrowser(QString("回调 : void commErrorHandler(void)"),QString(""));
}

void serialPortSender2(Uint8* pPackBuff, Uint16 packLen)
{
    pSerialPort2->sendData(pPackBuff,packLen);
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
    initStruct.pRecvBuff = recvBuff1;
    initStruct.recvBuffLen = RECV_BUFF1_LEN;
    initStruct.packBuffLen = 100;
    initStruct.dataBuffLen = 100;
    initStruct.callBack.pDataSender = serialPortSender1;
    initStruct.callBack.pDataHandler = dataHandler1;
    initStruct.callBack.pCommErrorHandler = commErrorHandler1;
    ModbusModule modbusInstance1(initStruct);
    modbusInstance1.setSerialPort(pSerialPort1);

    initStruct.timeOutTime = 50;
    initStruct.reSendTimes = 10;
    initStruct.pRecvBuff = recvBuff2;
    initStruct.recvBuffLen = RECV_BUFF2_LEN;
    initStruct.packBuffLen = 200;
    initStruct.dataBuffLen = 200;
    initStruct.callBack.pDataSender = serialPortSender2;
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
