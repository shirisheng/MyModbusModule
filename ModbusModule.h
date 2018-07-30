#ifndef MODBUSMODULE_H
#define MODBUSMODULE_H

#include <stdio.h>
#include <stdint.h>
#include <QMap>
#include <QString>
#include <QDebug>
#include "qglobal.h"
#include "SerialPortHelper.h"

/**********MODBUS CONFIG*********start*/
/*传输顺序：0:高字节在前，1:低字节在前*/
#define TRF_ORDER           0
/**********MODBUS CONFIG***********end*/

/**********MODBUS STATUS*********start*/
#define IDLE_STATUS          0x00
#define SEND_STATUS          0x01
#define SEND_FINISH          0x02
#define RECV_STATUS          0x03
#define RECV_FINISH          0x04
#define ERROR_STATUS         0x05
/**********MODBUS TATUS************end*/

/**********MODBUS FUNCODE********start*/
#define READ_MUL_COIL        0x01
#define READ_DSC_INPUT       0x02
#define READ_MUL_HLD_REG     0x03
#define READ_MUL_INPUT_REG   0x04
#define WRITE_ONE_COIL       0x05
#define WRITE_ONE_HLD_REG    0x06
#define WRITE_MUL_COIL       0x0FF
#define WRITE_MUL_HLD_REG    0x10
/**********MODBUS FUNCODE*********end*/

/**********TYPE DEFINE**********start*/
typedef int8_t           Sint8;
typedef uint8_t          Uint8;
typedef int16_t          Sint16;
typedef uint16_t         Uint16;
/**********TYPE DEFINE************end*/

/**********MODBUS ERROR*********start*/
enum MODBUS_ERROR
{
    Modbus_Error0 = 0,
    Modbus_Error1,      //打包缓冲区无效
    Modbus_Error2,      //打包缓冲区空间不足
    Modbus_Error3,      //接收缓冲区无效
    Modbus_Error4,      //接收缓冲区空间不足
    Modbus_Error5,      //定时器无效
    Modbus_Error6,      //CRC校验错误
    Modbus_Error7,      //从机返回异常响应帧
    Modbus_Error8,     //通信超时
};

enum EXCEPT_CODE
{ //从机返回异常响应帧中数据区对应的错误代码
    Except_Code1 = 1,
    Except_Code2,
    Except_Code3,
    Except_Code4,
    Except_Code5,
    Except_Code6,
    Except_Code7,
    Except_Code8,
};
/*异常响应帧数据区错误代码说明:
* 代码         名称                     含义
* 01      不合法功能代码     从机接收的是一种不能执行功能代码。
*						   发出查询命令后，该代码指示无程序功能。
* 02      不合法数据地址     接收的数据地址，是从机不允许的地址。
* 03      不合法数据        查询数据区的值是从机不允许的值。
* 04      从机设备故障      从机执行主机请求的动作时出现不可恢复的错误。
* 05      确认             从机已接收请求处理数据，但需要较长 的处理时
*						   间，为避免主机出现超时错误而发送该确认响应。
*						   主机以此再发送一个“查询程序完成”未决定从机
*						   是否已完成处理。
* 06      从机设备忙碌      从机正忙于处理一个长时程序命令，请求主机在
*						   从机空闲时发送信息。
* 07      否定             从机不能执行查询要求的程序功能时，该代码使用
*						  十进制 13 或 14 代码，向主机返回一个“不成功的
*						  编程请求”信息。主机应请求诊断从机的错误信息。
* 08      内存奇偶校验错误   从机读扩展内存中的数据时，发现有奇偶校验错误，
*						  主机按从机的要求重新发送数据请求
*/
/**********MODBUS ERROR************end*/

/**********COMMAND FRAME*********start*/
typedef struct _ReadMulRegCMD_X03_
{
    Uint8 slaveID;
    Uint8 funCode;
#if   (TRF_ORDER == 0)
    Uint8 startRegH;
    Uint8 startRegL;
    Uint8 regNumH;
    Uint8 regNumL;
#elif (TRF_ORDER == 1)
    Uint8 startRegL;
    Uint8 startRegH;
    Uint8 regNumL;
    Uint8 regNumH;
#endif
    Uint8 crcL;
    Uint8 crcH;
} ReadMulRegCMD_X03;

typedef struct _WriteOneRegCMD_X06_
{
    Uint8 slaveID;
    Uint8 funCode;
#if   (TRF_ORDER == 0)
    Uint8 startRegH;
    Uint8 startRegL;
    Uint8 dataH;
    Uint8 dataL;
#elif (TRF_ORDER == 1)
    Uint8 startRegL;
    Uint8 startRegH;
    Uint8 dataL;
    Uint8 dataH;
#endif
    Uint8 crcL;
    Uint8 crcH;
} WriteOneRegCMD_X06;

typedef struct _WriteMulRegCMD_X10_
{
    Uint8 slaveID;
    Uint8 funCode;
#if   (TRF_ORDER == 0)
    Uint8 startRegH;
    Uint8 startRegL;
    Uint8 regNumH;
    Uint8 regNumL;
    Uint8 dataLen;
    Uint8 data[5]; //此数组具体长度要视数据长度而定
#elif (TRF_ORDER == 1)
    Uint8 startRegL;
    Uint8 startRegH;
    Uint8 regNumL;
    Uint8 regNumH;
    Uint8 dataLen;
    Uint8 data[5]; //此数组具体长度要视数据长度而定
#endif
    Uint8 crcL;
    Uint8 crcH;
} WriteMulRegCMD_X10;

/**********COMMAND FRAME***********end*/

/**********RESPOND FRAME*********start*/
typedef struct _ReadMulRegRSP_X03_
{
    Uint8 slaveID;
    Uint8 funCode;
#if   (TRF_ORDER == 0)
    Uint8 dataLen;
    Uint8 data[5]; //此数组具体长度要视数据长度而定
#elif (TRF_ORDER == 1)
    Uint8 dataLenL;
    Uint8 dataLenH;
    Uint8 data[5]; //此数组具体长度要视数据长度而定
#endif
    Uint8 crcL;
    Uint8 crcH;
} ReadMulRegRSP_X03;

typedef struct _WriteOneRegRSP_X06_
{
    Uint8 slaveID;
    Uint8 funCode;
#if   (TRF_ORDER == 0)
    Uint8 startRegH;
    Uint8 startRegL;
    Uint8 dataH;
    Uint8 dataL;
#elif (TRF_ORDER == 1)
    Uint8 startRegL;
    Uint8 startRegH;
    Uint8 dataL;
    Uint8 dataH;
#endif
    Uint8 crcL;
    Uint8 crcH;
} WriteOneRegRSP_X06;

typedef struct _WriteMulRegRSP_X10_
{
    Uint8 slaveID;
    Uint8 funCode;
#if   (TRF_ORDER == 0)
    Uint8 startRegH;
    Uint8 startRegL;
    Uint8 regNumH;
    Uint8 regNumL;
    Uint8 dataLen;
    Uint8 data[5]; //此数组具体长度要视数据长度而定
#elif (TRF_ORDER == 1)
    Uint8 startRegL;
    Uint8 startRegH;
    Uint8 regNumL;
    Uint8 regNumH;
    Uint8 dataLen;
    Uint8 data[5]; //此数组具体长度要视数据长度而定
#endif
    Uint8 crcL;
    Uint8 crcH;
} WriteMulRegRSP_X10;

typedef struct _ExceptionRSP_
{
    Uint8 slaveID;
    Uint8 funCode; //异常响应功能码（其值为由当前功能码最高位置1所得）
    Uint8 except;  //异常的具体代码
    Uint8 crcL;
    Uint8 crcH;
} ExceptionRSP;
/**********RESPOND FRAME***********end*/

/**********MODBUS STRUCT*********start*/
typedef struct _ModbusPackStuct_
{
    Uint8* pPackBuff; //打包缓冲区（数据格式：包长+包数据）
    Uint16 pBuffLen;  //缓冲区长度
    Uint16 buffFront; //指向队首数据包
    Uint16 buffRear;  //指向队尾数据包
} ModbusPackStuct;

typedef struct _ModbusUnPackStuct_
{
    Uint8*  pRecvBuff; //接收缓冲区
    Uint16  rBuffLen;  //缓冲区长度
    Uint16  recvLen;   //接收总长度
    Uint16* pDataArea; //数据区指针
    Uint16  dataLen;   //数据区长度
} ModbusUnPackStuct;

typedef struct _ModbusRunInfo_
{
    Uint8  status;
    Uint8  exceptCode; //从机返回的错误码
    Uint8  error;      //错误类型
    Uint16 errTimes;   //错误次数
} ModbusRunInfo;

/*将pPackBuff中len字节的数据写入到通信设备中，返回实际写入的字节数*/
typedef Sint16 (*WriteData)(Uint8* pPackBuff, Uint16 len);

/*从通信设备中读取len字节的数据到pRecvBuff中，返回实际读取的字节数*/
typedef Sint16 (*ReadData)(Uint8* pRecvBuff, Uint16 len);

/*该函数在Modbus主机读取数据成功时被调用，使用者可于其中作相应的数据处理*/
typedef void   (*DataHandler)(Uint16* pDataArea, Uint16 len);

/*该函数在通信发生错误时被调用，使用者可于其中作相应的错误处理，错
 * 误处理完毕后，调用clearModbusError()以使Modbus模块继续工作*/
typedef void   (*ErrorHandler)(void);

typedef struct _ModbusCallBack_
{
    WriteData    pWriteData;    //写通信设备函数
    ReadData     pReadData;     //读通信设备函数
    DataHandler  pDataHandler;  //读数据处理函数
    ErrorHandler pErrorHandler; //通信错误处理函数
} ModbusCallBack;

typedef struct _ModbusInitStruct_
{
    Uint16* pMudbusTimer;  //Modbus定时器
    Uint16  timeOutTime;   //通信超时时间
    Uint16  reSendTimes;   //故障重发次数
    Uint16  recvBuffLen;   //接收缓冲区长度
    Uint16  packBuffLen;   //打包缓冲区长度
    ModbusCallBack callBack;
} ModbusInitStruct;
/**********MODBUS STRUCT***********end*/


class ModbusModule : public QObject
{
    Q_OBJECT
public:
    ModbusModule(ModbusInitStruct initStruct);
    ~ModbusModule();
    Uint8 readDataFromSlave(Uint8 slaveID, Uint16 startAddr, Uint16 dataNum);
    Uint8 writeDataToSlave(Uint8 slaveID, Uint16 addr, Uint16 data);
    /*错误处理完毕后，调用clearModbusError()以使Modbus模块继续工作*/
    void clearModbusError();
    /*该函数须需要循环调用，以使Modbus处于工作状态*/
    void runningModbus();
    ///////////////////////////////////////////////////////---
    Uint16 errorTimes(){return this->runInfo_.errTimes;}
    Uint16 packBuffSize(){return packStuct_.pBuffLen;}
    Uint16 packBuffFront(){return packStuct_.buffFront;}
    Uint16 packBuffRear(){return packStuct_.buffRear;}
    void setSerialPort(SerialPortHelper* pSerialPort);
public slots:
    void timeOutHandler();
    ///////////////////////////////////////////////////////---

private:
    Uint8 modbusInitCheck();
    Uint16 createCRC16(Uint8 *str,Uint16 num);
    Uint8 insertElement(Uint8* pBuff, Uint16* pFront, Uint16* pRear, Uint8 data);
    Uint8 deleteElement(Uint8* pBuff, Uint16* pFront, Uint16* pRear, Uint8* pData);
    Uint8 insertPack(Uint8* pBuff, Uint16* pFront, Uint16* pRear, Uint8* pPackData, Uint16 dataLen);
    Uint8 getCurrPack(Uint8* pBuff, Uint16* pFront, Uint16* pRear, Uint8* pPackData, Uint16* dataLen);
    Uint8 deleteCurrPack(Uint8* pBuff, Uint16* pFront, Uint16* pRear);
    Uint8 toFunCode03CmdPackRTU(Uint8 slaveID, Uint16 startReg,  Uint16 regNum);
    Uint8 toFunCode06CmdPackRTU(Uint8 slaveID, Uint16 startReg, Uint16 data);
    Uint8 addDataToRecvBuff(Uint8* pBuff, Uint16 len);
    Uint8 sendCurrCmdPackRTU();
    Uint8 getCmdPackHeader(Uint8* pHeader);
    Uint8 getRspPackHeader();
    Uint8 getRspPackLen(Uint8* rspPackLen);
    Uint8 receiveRspPackRTU();
    Uint8 handleRspPackRTU();
    void heartbeatDetect();
    void exceptRspHander(Uint8* pRecvBuff);
    void modbusErrorHandler();

private:
    Uint16 heartTimer_;    //心跳检测定时器
    Uint16* pMudbusTimer_; //超时检测定时器
    Uint16 timeOutTime_;   //通信超时时间
    Uint16 reSendTimes_;   //故障重发次数
    Uint16 reSendCount_;   //故障重发计数
    ModbusPackStuct packStuct_;
    ModbusUnPackStuct unPackStuct_;
    ModbusRunInfo runInfo_;
    ModbusCallBack callBack_;
    ///////////////////////////////////////////////////////---
    QMap<int,QString> errorToDecrMap_;
    SerialPortHelper* pSerialPort_;
    QTimer baseTimer_;
    Uint16 mudbusTimer_;
    ///////////////////////////////////////////////////////---
};

class HC_HydServoCtrl
{
public:
    HC_HydServoCtrl(ModbusInitStruct initStruct) { pModbus_ = new ModbusModule(initStruct);}
    ~HC_HydServoCtrl() { delete pModbus_;}
    ModbusModule* pModbus() { return pModbus_;}
    /*设置压力给定值*/
    Uint8 setPressGVal(Uint8 slaveID, Uint16 val) {return pModbus_->writeDataToSlave(slaveID, 0x02C0A, val);}
    /*读取压力给定值*/
    Uint8 getPressGVal(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, 0x3001, 1);}
    /*读取压力反馈值*/
    Uint8 getPressFVal(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, 0x3000, 1);}
    /*设置流量给定值*/
    Uint8 setFlowGVal(Uint8 slaveID, Uint16 val) {return pModbus_->writeDataToSlave(slaveID, 0x02C0B, val);}
    /*读取流量给定值*/
    Uint8 getFlowGVal(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, 0x3002, 1);}
    /*读取电机速度*/
    Uint8 getMtrSpeed(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, 0x3003, 1);}
    /*读取电机电流*/
    Uint8 getMtrCurrent(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, 0x3004, 1);}
    /*读取电机转矩*/
    Uint8 getMtrTorque(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, 0x3005, 1);}
    /*读取指令速度*/
    Uint8 getCmdSpeed(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, 0x3006, 1);}
    /*读取AI1模拟电压*/
    Uint8 getAI1Voltage(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, 0x3007, 1);}
    /*读取AI2模拟电压*/
    Uint8 getAI2Voltage(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, 0x3008, 1);}
    /*读取AI3模拟电压*/
    Uint8 getAI3Voltage(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, 0x3009, 1);}
    /*读取AI4模拟电压*/
    Uint8 getAI4Voltage(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, 0x300A, 1);}
    /*读取编码器位置*/
    Uint8 getEncoderPos(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, 0x300B, 1);}
    /*读取母线电压*/
    Uint8 getBusVoltage(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, 0x300C, 1);}
    /*读取驱动器温度*/
    Uint8 getTemperature(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, 0x300D, 1);}
    /*读取累计负载率*/
    Uint8 getCuLoadRate(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, 0x300F, 1);}
    /*读取再生制动负载率*/
    Uint8 getReLoadRate(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, 0x3010, 1);}
private:
    ModbusModule* pModbus_;
};

#endif // MODBUSMODULE_H
