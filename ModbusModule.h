#ifndef MODBUSMODULE_H
#define MODBUSMODULE_H

#include <stdio.h>
#include <stdint.h>
#include <QMap>
#include <QString>
#include <QDebug>
#include "qglobal.h"
#include "SerialPortHelper.h"

typedef int8_t   Int8;
typedef int16_t  Int16;
typedef uint8_t  Uint8;
typedef uint16_t Uint16;

/***************************************************MODBUS CONFIG*start*/
#define TRF_ORDER             0     //传输顺序：0:高字节在前，1:低字节在前

#if (TRF_ORDER == 0)
    #define HIGH_FIRST
#elif (TRF_ORDER == 1)
    #define LOW_FIRST
#endif
/*****************************************************MODBUS CONFIG*end*/


/************************************************MODBUS RUN STATUS*start*/
#define IDLE_STATUS          0x00
#define SEND_STATUS          0x01
#define SEND_FINISH          0x02
#define RECV_STATUS          0x03
#define RECV_FINISH          0x04
#define ERROR_STATUS         0x05
/*************************************************MODBUS RUN STATUS**end*/


/*******************************************************MODBUS CMD*start*/
#define READ_MUL_COIL        0x01
#define READ_DSC_INPUT       0x02
#define READ_MUL_HLD_REG     0x03
#define READ_MUL_INPUT_REG   0x04
#define WRITE_ONE_COIL       0x05
#define WRITE_ONE_HLD_REG    0x06
#define WRITE_MUL_COIL       0x0FF
#define WRITE_MUL_HLD_REG    0x10
/*********************************************************MODBUS CMD*end*/


/**********************************************MODBUS MODULE ERROR*start*/
enum MODBUS_ERROR
{
    Modbus_Error0 = 0,
    Modbus_Error1,      //从机地址错误
    Modbus_Error2,      //功能码不匹配
    Modbus_Error3,      //CRC校验错误
    Modbus_Error4,      //解包缓冲区无效
    Modbus_Error5,      //解包缓冲区空间不足
    Modbus_Error6,      //打包缓冲区无效
    Modbus_Error7,      //打包缓冲区空间不足
    Modbus_Error8,      //接收缓冲区无效
    Modbus_Error9,      //接收缓冲区空间不足
    Modbus_Error10,     //从机返回异常响应帧
    Modbus_Error11,     //功能码未实现
    Modbus_Error12,     //通信超时
    Modbus_Error13,     //定时器无效
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
*						  发出查询命令后，该代码指示无程序功能。
* 02      不合法数据地址     接收的数据地址，是从机不允许的地址。
* 03      不合法数据        查询数据区的值是从机不允许的值。
* 04      从机设备故障      从机执行主机请求的动作时出现不可恢复的错误。
* 05      确认             从机已接收请求处理数据，但需要较长 的处理时
*						  间，为避免主机出现超时错误而发送该确认响应。
*						  主机以此再发送一个“查询程序完成”未决定从机
*						  是否已完成处理。
* 06      从机设备忙碌      从机正忙于处理一个长时程序命令，请求主机在
*						  从机空闲时发送信息。
* 07      否定             从机不能执行查询要求的程序功能时，该代码使用
*						  十进制 13 或 14 代码，向主机返回一个“不成功的
*						  编程请求”信息。主机应请求诊断从机的错误信息。
* 08      内存奇偶校验错误   从机读扩展内存中的数据时，发现有奇偶校验错误，
*						  主机按从机的要求重新发送数据请求
*/
/***********************************************MODBUS MODULE ERROR*end*/


/************************************************MASTER CMD FRAME*start*/
typedef struct _ReadMulRegCMD_X03_
{
    Uint8 slaveID;
    Uint8 funCode;
#if defined(HIGH_FIRST)
    Uint8 startRegH;
    Uint8 startRegL;
    Uint8 regNumH;
    Uint8 regNumL;
#elif defined(LOW_FIRST)
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
#if defined(HIGH_FIRST)
    Uint8 startRegH;
    Uint8 startRegL;
    Uint8 dataH;
    Uint8 dataL;
#elif defined(LOW_FIRST)
    Uint8 startRegL;
    Uint8 startRegH;
    Uint8 dataL;
    Uint8 dataH;
#endif
    Uint8 crcL;
    Uint8 crcH;
} WriteMulRegCMD_X06;

typedef struct _WriteMulRegCMD_X10_
{
    Uint8 slaveID;
    Uint8 funCode;
#if defined(HIGH_FIRST)
    Uint8 startRegH;
    Uint8 startRegL;
    Uint8 regNumH;
    Uint8 regNumL;
    Uint8 dataLen;
    Uint8 data[5]; //此数组具体长度要视数据长度而定
#elif defined(LOW_FIRST)
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

/***************************************************MASTER CMD FRAME*end*/


/**********************************************SLAVE RESPOND FRAME*start*/
typedef struct _ReadMulRegRSP_X03_
{
    Uint8 slaveID;
    Uint8 funCode;
#if defined(HIGH_FIRST)
    Uint8 dataLen;
    Uint8 data[5]; //此数组具体长度要视数据长度而定
#elif defined(LOW_FIRST)
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
#if defined(HIGH_FIRST)
    Uint8 startRegH;
    Uint8 startRegL;
    Uint8 dataH;
    Uint8 dataL;
#elif defined(LOW_FIRST)
    Uint8 startRegL;
    Uint8 startRegH;
    Uint8 dataL;
    Uint8 dataH;
#endif
    Uint8 crcL;
    Uint8 crcH;
} WriteMulRegRSP_X06;

typedef struct _WriteMulRegRSP_X10_
{
    Uint8 slaveID;
    Uint8 funCode;
#if defined(HIGH_FIRST)
    Uint8 startRegH;
    Uint8 startRegL;
    Uint8 regNumH;
    Uint8 regNumL;
    Uint8 dataLen;
    Uint8 data[5]; //此数组具体长度要视数据长度而定
#elif defined(LOW_FIRST)
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
    Uint8 funCode; //异常响应功能码的生成方式:在当前功能码的基础上将最高位置1
    Uint8 except; //异常的具体代码
    Uint8 crcL;
    Uint8 crcH;
} ExceptionRSP;
/************************************************MASTER RESPOND FRAME*end*/


/***********************************************MODBUS MODULE STRUCT*start*/
typedef struct _ModbusPackStuct_
{
    Uint8* pPackBuff; //打包缓冲区（数据格式：包长+包数据）
    Uint16 pBuffLen; //缓冲区长度
    Uint16 buffFront; //指向队首数据包
    Uint16 buffRear; //指向队尾数据包
} ModbusPackStuct;

typedef struct _ModbusUnPackStuct_
{
    Uint8* pRecvBuff; //接收缓冲区
    Uint16 rBuffLen; //缓冲区长度
    Uint16 recvLen; //接收数据长度
    Uint16* pDataBuff; //解包据数缓冲区
    Uint16 dBuffLen; //缓冲区长度
    Uint16 dataLen; //数据长度
} ModbusUnPackStuct;

typedef struct _ModbusRunInfo_
{
    Uint8 status;
    Uint8 exceptCode; //从机返回异常帧时，数据区对应的错误码
    Uint8 error;      //错误类型
} ModbusRunInfo;

typedef Int16 (*DataSender)(Uint8* pPackBuff, Uint16 packLen);
typedef Int16 (*DataReceiver)(Uint8* pRecvBuff, Uint16 recvLen);
typedef void (*DataHandler)(Uint16* pUnPackData, Uint16 dataLen);
typedef void (*CommErrorHandler)(void);

typedef struct _ModbusCallBackStruct_
{
    DataSender pDataSender;
    DataReceiver pDataReceiver;
    DataHandler pDataHandler;
    CommErrorHandler pCommErrorHandler;
} ModbusCallBackStruct;

typedef struct _ModbusInitStruct_
{
    Uint16* pMudbusTimer;
    Uint16 timeOutTime; //通信超时时间
    Uint16 reSendTimes; //故障重发次数
    Uint16 recvBuffLen; //接收缓冲区长度
    Uint16 packBuffLen; //打包缓冲区长度
    Uint16 dataBuffLen; //解包缓冲区长度
    ModbusCallBackStruct callBack;
} ModbusInitStruct;
/***********************************************MODBUS MODULE STRUCT*end*/


class ModbusModule : public QObject
{
    Q_OBJECT
public:
    ModbusModule(ModbusInitStruct initStruct);
    ~ModbusModule();
    bool readDataFromSlave(Uint8 slaveID, Uint16 startAddr, Uint16 dataNum);
    bool writeDataToSlave(Uint8 slaveID, Uint16 startAddr, Uint16 data);
    void clearErrAndSendNextPack();
    void modbusRunningController();

    Uint16 packBuffSize(){return packStuct_.pBuffLen;}
    Uint16 packBuffFront(){return packStuct_.buffFront;}
    Uint16 packBuffRear(){return packStuct_.buffRear;}
    void setSerialPort(SerialPortHelper* pSerialPort);

public slots:
    void timeOutHandler();
//    void notifyModbusRecvFinish(Uint8* pUPckBuff, Uint16 buffLen);

private:
    Uint8 modbusHasInitCheck();
    Uint16 createCRC16(Uint8 *str,Uint16 num);
    Uint8 insertElement(Uint8* pBuff, Uint16* pFront, Uint16* pRear, Uint8 data);
    Uint8 deleteElement(Uint8* pBuff, Uint16* pFront, Uint16* pRear, Uint8* pData);
    Uint8 insertPack(Uint8* pBuff, Uint16* pFront, Uint16* pRear, Uint8* pPackData, Uint16 dataLen);
    Uint8 getCurrPack(Uint8* pBuff, Uint16* pFront, Uint16* pRear, Uint8* pPackData, Uint16* dataLen);
    Uint8 deleteCurrPack(Uint8* pBuff, Uint16* pFront, Uint16* pRear);
    bool toFunCode03CmdPackRTU(Uint8 slaveID, Uint16 startReg,  Uint16 regNum);
    bool funCode03RspUnPackRTU();
    bool toFunCode06CmdPackRTU(Uint8 slaveID, Uint16 startReg, Uint16 data);
    bool funCode06RspUnPackRTU();
    void exceptRspHander(Uint8* pUPkSrcBuff);
    Uint8 addDataToRecvBuff(Uint8* pBuff, Uint16 len);
    Uint8 sendCurrCmdPackRTU();
    Uint8 captureRspPackRTU();
    bool modbusRspUnPackRTU(Uint8* pUPkSrcBuff);
    void clearModbusError();
    void modbusErrorHandler();

private:
    Uint16* pMudbusTimer_; //定时器
    Uint16 timeOutTime_; //通信超时时间
    Uint16 reSendTimes_; //故障重发次数
    Uint16 reSendCount_; //重发计数
    ModbusPackStuct packStuct_;
    ModbusUnPackStuct unPackStuct_;
    ModbusRunInfo runInfo_;
    ModbusCallBackStruct callBackStruct_;
    ///////////////////////////////////////////
    QMap<int,QString> errorToDecrMap_;
    SerialPortHelper* pSerialPort_;
    QTimer baseTimer_;
    Uint16 mudbusTimer_;
    ///////////////////////////////////////////
};

#endif // MODBUSMODULE_H
