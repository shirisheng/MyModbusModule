#ifndef MODBUSSLAVE_H
#define MODBUSSLAVE_H

#include "ModbusComDef.h"

/// @name Slave Status
/// @{
#define SLAVE_IDLE_STATUS          0x00
#define SLAVE_RSPD_STATUS          0x01
#define SLAVE_RSPD_FINISH          0x02
#define SLAVE_RECV_STATUS          0x03
#define SLAVE_RECV_FINISH          0x04
#define SLAVE_ERROR_STATUS         0x05
/// @} End of Slave Status

/// @name Slave Error
/// @{
enum SLAVE_ERROR
{
    SLAVE_Error0 = 0,
    SLAVE_Error1,      ///< 初始化失败
    SLAVE_Error2,      ///< 发送缓冲区空间不足
    SLAVE_Error3,      ///< 接收缓冲区空间不足
    SLAVE_Error4,      ///< 读寄存器缓冲区不足
    SLAVE_Error5,      ///< CRC校验错误
	SLAVE_Error6,      ///< 接收长度错误
};
/// @} End of Slave Error

/// @name Slave struct
/// @{
typedef struct
{
    Uint8*  pBuff;     ///< 发送缓冲区
    Uint16  buffLen;   ///< 缓冲区长度
    Uint16  sendLen;   ///< 发送总长度
    Uint16  hasSend;   ///< 已发送长度
} SlaveSendBuff;

typedef struct
{
    Uint8*  pBuff;     ///< 接收缓冲区
    Uint16  buffLen;   ///< 缓冲区长度
    Uint16  recvLen;   ///< 接收总长度
    Uint16  hasRecv;   ///< 已接收长度
} SlaveRecvBuff;

typedef struct
{
#ifndef DEBUG_CODE
    ///< @brief 从保持寄存器中读取len个字节的数据到buff，返回实际读取到的字节数
    Sint16 (*readRegister)(Uint16* buff, Uint16 addr, Uint16 num);
    ///< @brief 将buff中的len个字节的数据写入到保持寄存器，返回实际写入的字节数
    Sint16 (*writeRegister)(Uint16* buff, Uint16 addr, Uint16 num);
#endif
    /// @brief 从通信设备中读取len字节的数据到pBuff中，读取成功时返回实际读取的字节数，否则返回-1
    Sint16 (*readComDev)(Uint8* buff, Uint16 len);
    /// @brief 将pBuff中len字节的数据写入到通信设备中，写入成功时返回实际写入的字节数，否则返回-1
    Sint16 (*writeComDev)(Uint8* buff, Uint16 len);
    ///< @brief 该函数用于判断通信设备是否接收到数据
    Uint8  (*hasDataInComDev)(void);
} SlaveCallBack;

typedef struct
{
    Uint16* pTimeCounter;    ///< 定时计数器
    Uint16  countCyclTime;   ///< 计数周期ms
} SlaveBaseTimer;

typedef struct
{
    Uint8  status;
    Uint8  error;      ///< 错误类型
    Uint16 errTimes;   ///< 错误次数
} SlaveRunInfo;

typedef struct
{
    Uint8   slaveID;           ///< 当前使用的ID
    Uint32  baudRate;          ///< 当前使用波特率
    Uint16* pTimeCounter;      ///< 定时计数器
    Uint16  countCyclTime;     ///< 计数周期ms
    Uint16  sendBuffLen;       ///< 发送缓冲区长度
    Uint16  recvBuffLen;       ///< 接收缓冲区长度
    SlaveCallBack callBack;
} SlaveInitStruct;
/// @} End of Slave Struct

/// @brief Slave通信模块类
class ModbusSlave
#ifdef DEBUG_CODE
    : public QObject
#endif
{
#ifdef DEBUG_CODE
    Q_OBJECT
#endif
public:
    ModbusSlave(SlaveInitStruct initStruct);
    ~ModbusSlave();
    /// @brief 运行Modbus Slave（该函数须需要定时循环调用）
    void runModbusSlave();
    /// @brief 清除错误，使Slave退出错误状态
    void clearSlaveError();
    Uint16 hasRecvLen() { return recvBuff_.hasRecv;}
    Uint16 hasSendLen() { return sendBuff_.hasSend;}
    void setHasRecvLen(Uint16 len) { recvBuff_.hasRecv = len;}
    void setHasSendLen(Uint16 len) { sendBuff_.hasSend = len;}
    Uint16 errorTimes() {return runInfo_.errTimes;}
#ifdef DEBUG_CODE
    void setSerialPort(SerialPortHelper* pSerialPort);
public slots:
    void timeOutHandler();
#endif
private:
    Uint16 createCRC16(Uint8 *str,Uint16 num);
    Bool isInitSuccess() { return isInitOK_;}
    void baseTimerHandler();
    Bool isCmdPackComeIn();
    void prepareForRecv();
    Bool recvCmdPackRTU();
    void createGapTime(Uint32 baudRate);
    Bool isSlaveIDValid();
    Bool isSendBuffEnough(Uint16 sendLen);
    Bool funCode03CmdPackHandle();
    Bool funCode06CmdPackHandle();
    Bool createExpRspPack(Uint8 exceptCode);
    Bool masterCmdPackHandle();
    void slaveErrorHandler();
    void prepareForSend();
    Bool sendRspPackRTU();
    Sint16 readRegister(Uint16* buff, Uint16 addr, Uint16 num);
    Sint16 writeRegister(Uint16* buff, Uint16 addr, Uint16 num);
private:
    Bool   isInitOK_;  ///< 初始化成功
    Uint8  slaveID_;   ///< 当前使用的ID
    Uint32 baudRate_;  ///< 当前波特率
    double rGapTime_;  ///< 接收间隔时间
    double fGapTime_;  ///< 帧间隔时间ms
    SlaveSendBuff  sendBuff_;
    SlaveRecvBuff  recvBuff_;
    SlaveCallBack  callBack_;
    SlaveBaseTimer baseTimer_;
    SlaveRunInfo   runInfo_;
#ifdef DEBUG_CODE
    Uint16  memSpace[500];///< 从机虚拟存储空间
    SerialPortHelper* pSerialPort_;
    QMap<int,QString> errorToDecrMap_;
    QTimer debugTimer_;
    Uint16 timerCounter_;
#endif
};

#endif // MODBUSSLAVE_H
