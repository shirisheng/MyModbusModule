#include "mainwindow.h"
#include <QApplication>
#include "ModbusMaster.h"
#include "ModbusSlave.h"
#include "SerialPortHelper.h"

SerialPortHelper* pSerialPort1;
SerialPortHelper* pSerialPort2;
/********************************************************start*/
Sint16 writeSerial1(Uint8* pBuff, Uint16 len)
{
    return pSerialPort1->writeSerial(pBuff,len);
}

Sint16 readSerial1(Uint8* pBuff, Uint16 len)
{
    return pSerialPort1->readSerial(pBuff,len);
}

Uint8 serial1HasData()
{
    return pSerialPort1->currentSerialPort()->bytesAvailable() != 0;
}

void dataHandler1(Uint16* pData, Uint16 len, Uint16 descr)
{
    pSerialPort1->showInCommBrowser(QString("void (*dataHandler)(Uint16* pData, Uint16 len, Uint16 descr)"),QString(""));
}

void errorHandler1(void)
{
    pSerialPort1->showInCommBrowser(QString("void (*errorHandler)(void)"),QString(""));
}

Sint16 writeSerial2(Uint8* pBuff, Uint16 len)
{
    return pSerialPort2->writeSerial(pBuff,len);
}

Sint16 readSerial2(Uint8* pBuff, Uint16 len)
{
    return pSerialPort2->readSerial(pBuff,len);
}

Uint8 serial2HasData()
{
    return pSerialPort2->currentSerialPort()->bytesAvailable() != 0;
}

void dataHandler2(Uint16* pData, Uint16 len, Uint16 descr)
{
    pSerialPort2->showInCommBrowser(QString("void (*dataHandler)(Uint16* pData, Uint16 len, Uint16 descr)"),QString(""));
}

void errorHandler2(void)
{
    pSerialPort2->showInCommBrowser(QString("void (*errorHandler)(void)"),QString(""));
}
/**********************************************************end*/
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    pSerialPort1 = new SerialPortHelper;
    pSerialPort2 = new SerialPortHelper;
#ifdef DEBUG_CODE
    /// @name Modbus主机调试代码
    /// @{
    MasterInitStruct masterInitSt;
    masterInitSt.commTimeOut = 2000;
    masterInitSt.reSendTimes = 10;
    masterInitSt.recvBuffLen = 200;
    masterInitSt.sendBuffLen = 100;
    masterInitSt.callBack.writeComDev = writeSerial1;
    masterInitSt.callBack.readComDev = readSerial1;
    masterInitSt.callBack.dataHandler = dataHandler1;
    masterInitSt.callBack.errorHandler = errorHandler1;
    ModbusMaster modbusMaster(masterInitSt);
    modbusMaster.setSerialPort(pSerialPort1);
    /// @} End of Modbus主机调试代码
    /// @name Modbus从机调试代码
    /// @{
    SlaveInitStruct slaveInitSt;
    slaveInitSt.slaveID = 0x01;
    slaveInitSt.baudRate = 9600;
    slaveInitSt.sendBuffLen = 200;
    slaveInitSt.recvBuffLen = 200;
    slaveInitSt.callBack.readComDev = readSerial2;
    slaveInitSt.callBack.writeComDev = writeSerial2;
    slaveInitSt.callBack.hasDataInComDev = serial2HasData;
    ModbusSlave modbusSlave(slaveInitSt);
    modbusSlave.setSerialPort(pSerialPort2);
    /// @} End of Modbus从机调试代码
    pSerialPort1->setWindowTitle("Master Serial Helper");
    pSerialPort2->setWindowTitle("Slave Serial Helper");
    pSerialPort1->show();
    pSerialPort2->show();
    MainWindow w1;
    w1.setModbusMaster(&modbusMaster);
    w1.setModbusSlave(&modbusSlave);
    w1.setWindowTitle("Master Debuger");
    w1.show();
#endif
    return a.exec();
}
