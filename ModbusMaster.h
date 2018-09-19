#ifndef MODBUSMASTER_H
#define MODBUSMASTER_H

#include "modbuscomdef.h"

/// @name Master Status
/// @{
#define MASTER_IDLE_STATUS          0x00
#define MASTER_SEND_STATUS          0x01
#define MASTER_SEND_FINISH          0x02
#define MASTER_RECV_STATUS          0x03
#define MASTER_RECV_FINISH          0x04
#define MASTER_ERROR_STATUS         0x05
/// @} End of Master Status

/// @name Master Error
/// @{
enum MASTER_ERROR
{
    Master_Error0 = 0,
    Master_Error1,      ///< 初始化失败
    Master_Error2,      ///< 发送缓冲区空间不足
    Master_Error3,      ///< 接收缓冲区空间不足
    Master_Error4,      ///< CRC校验错误
    Master_Error5,      ///< 从机返回异常响应帧
    Master_Error6,      ///< 通信接收超时
    Master_Error7,      ///< 获取当前数据包失败
};
/// @} End of Master Error

/// @name Master Struct
/// @{
typedef struct
{
    Uint8* pBuff;    ///< 发送缓冲区（数据格式: 包长+包数据）
    Uint16 buffLen;  ///< 缓冲区长度
    Uint16 front;    ///< 指向队首数据包
    Uint16 rear;     ///< 指向队尾数据包
} MasterSendBuff;

typedef MasterSendBuff SendQueue;

typedef struct
{
    Uint8*  pBuff;      ///< 接收缓冲区
    Uint16  buffLen;    ///< 缓冲区长度
    Uint16  recvLen;    ///< 接收总长度
    Uint16  hasRecv;    ///< 已接收长度
    Uint16* pDArea;     ///< 数据区指针
    Uint16  dataLen;    ///< 数据区长度
    Uint16  dataDecr;   ///< 数据描述（addr）
} MasterRecvBuff;

typedef struct
{
    /// @brief 从通信设备中读取len字节的数据到pBuff中，读取成功时返回实际读取的字节数，否则返回-1
    Sint16 (*readComDev)(Uint8* pBuff, Uint16 len);
    /// @brief 将pBuff中len字节的数据写入到通信设备中，写入成功时返回实际写入的字节数，否则返回-1
    Sint16 (*writeComDev)(Uint8* pBuff, Uint16 len);
    /// @brief 数据处理函数，该函数在Modbus主机读取数据成功时被调用，使用者可于其中作相应的数据处理
    void (*dataHandler)(Uint16* pData, Uint16 len, Uint16 descr);
    /// @brief 错误处理函数，该函数在通信发生错误时被调用，使用者可于其中作相应的错
    /// 误处理，错误处理完毕后，调用clearModbusError()以使Modbus模块退出错误状态
    void (*errorHandler)(void);
} MasterCallBack;

typedef struct
{
    Uint16* pTimerCounter;   ///< 定时计数器
    Uint16  countCyclTime;   ///< 计数周期ms
} MasterBaseTimer;

typedef struct
{
    Uint8  status;
    Uint8  exceptCode; ///< 从机返回的错误码
    Uint8  error;      ///< 错误类型
    Uint16 errTimes;   ///< 错误次数
} MasterRunInfo;

typedef struct
{
    Uint16* pTimerCounter; ///< 定时计数器
    Uint16  countCyclTime; ///< 计数周期ms
    Uint16  commTimeOut;   ///< 通信超时时间ms
    Uint16  reSendTimes;   ///< 故障重发次数
    Uint16  recvBuffLen;   ///< 接收缓冲区长度
    Uint16  sendBuffLen;   ///< 发送缓冲区长度
    MasterCallBack callBack;
} MasterInitStruct;
/// @} End of Master Struct


/// @brief Master通信模块类
class ModbusMaster
#ifdef DEBUG_CODE
    : public QObject
#endif
{
#ifdef DEBUG_CODE
    Q_OBJECT
#endif
public:
    ModbusMaster(MasterInitStruct initStruct);
    ~ModbusMaster();
    Bool readDataFromSlave(Uint8 slaveID, Uint16 startAddr, Uint16 dataNum);
    Bool writeDataToSlave(Uint8 slaveID, Uint16 addr, Uint16 data);
    /// @brief 运行Modbus Master（该函数须需要定时循环调用）
    void runModbusMaster();
    /// @brief 清除错误，使Master退出错误状态（该函数须在错误处理函数中调用）
    void clearMasterError();
#ifdef DEBUG_CODE
    Uint16 errorTimes(){return this->runInfo_.errTimes;}
    Uint16 sendBuffSize(){return sendBuff_.buffLen;}
    Uint16 sendBuffFront(){return sendBuff_.front;}
    Uint16 sendBuffRear(){return sendBuff_.rear;}
    void   setSerialPort(SerialPortHelper* pSerialPort);
public slots:
    void timeOutHandler();
#endif

private:
    Bool isInitSuccess() { return isInitOK_;}
    void  baseTimerHandler();
    Uint16 createCRC16(Uint8 *str,Uint16 num);
    Bool isSendBuffEmpty();
    Bool insertElement(SendQueue* pQueue, Uint8 data);
    Bool deleteElement(SendQueue* pQueue, Uint8* pPack);
    Bool insertPack(SendQueue* pQueue, Uint8* pPack, Uint16 packLen);
    Bool getCurrPack(SendQueue* pQueue, Uint8* pPack, Uint16* pPackLen);
    Bool deleteCurrPack(SendQueue* pQueue);
    Bool toFunCode03CmdPackRTU(Uint8 slaveID, Uint16 startReg,  Uint16 regNum);
    Bool toFunCode06CmdPackRTU(Uint8 slaveID, Uint16 startReg, Uint16 data);
    Bool sendCurrCmdPackRTU();
    Bool getCmdPackHeader(Uint8* pHeader);
    Bool getRspPackHeader();
    Bool getRspPackLen(Uint8* pPackLen);
    Bool recvRspPackRTU();
    Sint16 getDataDescr();
    Bool handleRspPackRTU();
    void heartbeatDetect();
    void exceptRspHander(Uint8* pRecvBuff);
    void masterErrorHandler();
    void appErrorHandler();

private:
    Bool  isInitOK_;       ///< 初始化成功
    Uint16 heartChkTime_;  ///< 心跳检测计时
    Uint16 currRecvTime_;  ///< 当前接收耗时
    Uint16 commTimeOut_;   ///< 通信超时时间ms
    Uint16 reSendTimes_;   ///< 故障重发次数
    Uint16 reSendCount_;   ///< 故障重发计数
    MasterBaseTimer baseTimer_;
    MasterSendBuff  sendBuff_;
    MasterRecvBuff  recvBuff_;
    MasterRunInfo   runInfo_;
    MasterCallBack  callBack_;
#ifdef DEBUG_CODE
    QMap<int,QString> errorToDecrMap_;
    SerialPortHelper* pSerialPort_;
    QTimer debugTimer_;
    Uint16 timerCounter_;
#endif
};

/// @brief 华成液压伺服控制类.
class HC_HydServoCtrl
{
    enum REDA_ITEM_DECR
    {
        R_PressFVal   = 0x3000,
        R_PressGVal   = 0x3001,
        R_FlowGVal    = 0x3002,
        R_MtrSpeed    = 0x3003,
        R_MtrCurrent  = 0x3004,
        R_MtrTorque   = 0x3005,
        R_CmdSpeed    = 0x3006,
        R_AI1Voltage  = 0x3007,
        R_AI2Voltage  = 0x3008,
        R_AI3Voltage  = 0x3009,
        R_AI4Voltage  = 0x300A,
        R_EncoderPos  = 0x300B,
        R_BusVoltage  = 0x300C,
        R_Temperature = 0x300D,
        R_CuLoadRate  = 0x300F,
        R_ReLoadRate  = 0x3010,
    };

    enum WRITE_ITEM_DECR
    {
        W_PressGVal   = 0x2C0A,
        W_FlowGVal    = 0x2C0B,
    };

public:
    HC_HydServoCtrl(MasterInitStruct initStruct) { pModbus_ = new ModbusMaster(initStruct);}
    ~HC_HydServoCtrl() { delete pModbus_;}
    /// @brief 返回Modbus实例指针
    ModbusMaster* pModbus() { return pModbus_;}
    /// @brief 设置压力给定值
    Uint8 setPressGVal(Uint8 slaveID, Uint16 val) {return pModbus_->writeDataToSlave(slaveID, W_PressGVal, val);}
    /// @brief 读取压力给定值
    Uint8 getPressGVal(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, R_PressGVal, 1);}
    /// @brief 读取压力反馈值
    Uint8 getPressFVal(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, R_PressFVal, 1);}
    /// @brief 设置流量给定值
    Uint8 setFlowGVal(Uint8 slaveID, Uint16 val) {return pModbus_->writeDataToSlave(slaveID, W_FlowGVal, val);}
    /// @brief 读取流量给定值
    Uint8 getFlowGVal(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, R_FlowGVal, 1);}
    /// @brief 读取电机速度
    Uint8 getMtrSpeed(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, R_MtrSpeed, 1);}
    /// @brief 读取电机电流
    Uint8 getMtrCurrent(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, R_MtrCurrent, 1);}
    /// @brief 读取电机转矩
    Uint8 getMtrTorque(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, R_MtrTorque, 1);}
    /// @brief 读取指令速度
    Uint8 getCmdSpeed(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, R_CmdSpeed, 1);}
    /// @brief 读取AI1模拟电压
    Uint8 getAI1Voltage(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, R_AI1Voltage, 1);}
    /// @brief 读取AI2模拟电压
    Uint8 getAI2Voltage(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, R_AI2Voltage, 1);}
    /// @brief 读取AI3模拟电压
    Uint8 getAI3Voltage(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, R_AI3Voltage, 1);}
    /// @brief 读取AI4模拟电压
    Uint8 getAI4Voltage(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, R_AI4Voltage, 1);}
    /// @brief 读取编码器位置
    Uint8 getEncoderPos(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, R_EncoderPos, 1);}
    /// @brief 读取母线电压
    Uint8 getBusVoltage(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, R_BusVoltage, 1);}
    /// @brief 读取驱动器温度
    Uint8 getTemperature(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, R_Temperature, 1);}
    /// @brief 读取累计负载率
    Uint8 getCuLoadRate(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, R_CuLoadRate, 1);}
    /// @brief 读取再生制动负载率
    Uint8 getReLoadRate(Uint8 slaveID) {return pModbus_->readDataFromSlave(slaveID, R_ReLoadRate, 1);}
private:
    ModbusMaster* pModbus_;
};

#endif // MODBUSMASTER_H
