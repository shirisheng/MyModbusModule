#ifndef MODBUSCOMDEF_H
#define MODBUSCOMDEF_H

#define DEBUG_CODE
#include <stdio.h>
#include <stdint.h>
#ifdef DEBUG_CODE
#include <QMap>
#include <QString>
#include <QDebug>
#include "qglobal.h"
#include "SerialPortHelper.h"
#endif

/// @name Modbus Config
/// @{
/// @brief 传输顺序：0:高字节在前，1:低字节在前
#define TRF_ORDER                   0
/// @} End of Modbus Config


/// @name Modbus FunCode
/// @{
#define READ_MUL_COIL              0x01
#define READ_DSC_INPUT             0x02
#define READ_MUL_HLD_REG           0x03
#define READ_MUL_INPUT_REG         0x04
#define WRITE_ONE_COIL             0x05
#define WRITE_ONE_HLD_REG          0x06
#define WRITE_MUL_COIL             0x0FF
#define WRITE_MUL_HLD_REG          0x10
/// @} End of Modbus FunCode


/// @name Type Define
/// @{
#define TRUE              1
#define FALSE             0
typedef unsigned char    Bool;
typedef char             Sint8;
typedef unsigned char    Uint8;
typedef int16_t          Sint16;
typedef uint16_t         Uint16;
typedef int32_t          Sint32;
typedef uint32_t         Uint32;
/// @} End of Type Define

/// @name Modbus Except Code
/// @{
enum ExceptCode
{ /// 从机返回异常响应帧中数据区对应的错误代码
    Except_Code0 = 0,
    Except_Code1,
    Except_Code2,
    Except_Code3,
    Except_Code4,
    Except_Code5,
    Except_Code6,
    Except_Code7,
    Except_Code8,
};
/// 异常响应帧数据区错误代码说明:
/// 代码         名称                     含义
/// 01      不合法功能代码    从机接收的是一种不能执行功能代码。
///						    发出查询命令后，该代码指示无程序功能。
/// 02      不合法数据地址    接收的数据地址，是从机不允许的地址。
/// 03      不合法数据       查询数据区的值是从机不允许的值。
/// 04      从机设备故障     从机执行主机请求的动作时出现不可恢复的错误。
/// 05      确认            从机已接收请求处理数据，但需要较长 的处理时
///						    间，为避免主机出现超时错误而发送该确认响应。
///						    主机以此再发送一个“查询程序完成”未决定从机
///						    是否已完成处理。
/// 06      从机设备忙碌     从机正忙于处理一个长时程序命令，请求主机在
///						    从机空闲时发送信息。
/// 07      否定            从机不能执行查询要求的程序功能时，该代码使用
///						    十进制 13 或 14 代码，向主机返回一个“不成功的
///						    编程请求”信息。主机应请求诊断从机的错误信息。
/// 08      内存奇偶校验错误  从机读扩展内存中的数据时，发现有奇偶校验错误，
///						    主机按从机的要求重新发送数据请求
/// @} End of Modbus Except Code



/// @name Modbus Command Frame
/// @{
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
/// @brief 此数组具体长度要视数据长度而定
    Uint8 data[5];
#elif (TRF_ORDER == 1)
    Uint8 startRegL;
    Uint8 startRegH;
    Uint8 regNumL;
    Uint8 regNumH;
    Uint8 dataLen;
/// @brief 此数组具体长度要视数据长度而定
    Uint8 data[5];
#endif
    Uint8 crcL;
    Uint8 crcH;
} WriteMulRegCMD_X10;
/// @} End of Modbus Command Frame

/// @name Modbus Respond Frame
/// @{
typedef struct _ReadMulRegRSP_X03_
{
    Uint8 slaveID;
    Uint8 funCode;
#if   (TRF_ORDER == 0)
    Uint8 dataLen;
/// @brief 此数组具体长度要视数据长度而定
    Uint8 data[5];
#elif (TRF_ORDER == 1)
    Uint8 dataLenL;
    Uint8 dataLenH;
/// @brief 此数组具体长度要视数据长度而定
    Uint8 data[5];
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
/// @brief 此数组具体长度要视数据长度而定
    Uint8 data[5];
#elif (TRF_ORDER == 1)
    Uint8 startRegL;
    Uint8 startRegH;
    Uint8 regNumL;
    Uint8 regNumH;
    Uint8 dataLen;
/// @brief 此数组具体长度要视数据长度而定
    Uint8 data[5];
#endif
    Uint8 crcL;
    Uint8 crcH;
} WriteMulRegRSP_X10;

typedef struct _ExceptionRSP_
{
    Uint8 slaveID;
/// @brief 异常功能码（其值为由当前功能码最高位置1所得）
    Uint8 funCode;
/// @brief 异常类型码
    Uint8 except;
    Uint8 crcL;
    Uint8 crcH;
} ExceptionRSP;
/// @} End of Modbus Respond Frame

#ifndef CRCTABLE
#define CRCTABLE
const Uint16 crctable[] =
{
        0x0000,0xC0C1,0xC181,0x0140,0xC301,0x03C0,0x0280,0xC241,
        0xC601,0x06C0,0x0780,0xC741,0x0500,0xC5C1,0xC481,0x0440,
        0xCC01,0x0CC0,0x0D80,0xCD41,0x0F00,0xCFC1,0xCE81,0x0E40,
        0x0A00,0xCAC1,0xCB81,0x0B40,0xC901,0x09C0,0x0880,0xC841,
        0xD801,0x18C0,0x1980,0xD941,0x1B00,0xDBC1,0xDA81,0x1A40,
        0x1E00,0xDEC1,0xDF81,0x1F40,0xDD01,0x1DC0,0x1C80,0xDC41,
        0x1400,0xD4C1,0xD581,0x1540,0xD701,0x17C0,0x1680,0xD641,
        0xD201,0x12C0,0x1380,0xD341,0x1100,0xD1C1,0xD081,0x1040,
        0xF001,0x30C0,0x3180,0xF141,0x3300,0xF3C1,0xF281,0x3240,
        0x3600,0xF6C1,0xF781,0x3740,0xF501,0x35C0,0x3480,0xF441,
        0x3C00,0xFCC1,0xFD81,0x3D40,0xFF01,0x3FC0,0x3E80,0xFE41,
        0xFA01,0x3AC0,0x3B80,0xFB41,0x3900,0xF9C1,0xF881,0x3840,
        0x2800,0xE8C1,0xE981,0x2940,0xEB01,0x2BC0,0x2A80,0xEA41,
        0xEE01,0x2EC0,0x2F80,0xEF41,0x2D00,0xEDC1,0xEC81,0x2C40,
        0xE401,0x24C0,0x2580,0xE541,0x2700,0xE7C1,0xE681,0x2640,
        0x2200,0xE2C1,0xE381,0x2340,0xE101,0x21C0,0x2080,0xE041,
        0xA001,0x60C0,0x6180,0xA141,0x6300,0xA3C1,0xA281,0x6240,
        0x6600,0xA6C1,0xA781,0x6740,0xA501,0x65C0,0x6480,0xA441,
        0x6C00,0xACC1,0xAD81,0x6D40,0xAF01,0x6FC0,0x6E80,0xAE41,
        0xAA01,0x6AC0,0x6B80,0xAB41,0x6900,0xA9C1,0xA881,0x6840,
        0x7800,0xB8C1,0xB981,0x7940,0xBB01,0x7BC0,0x7A80,0xBA41,
        0xBE01,0x7EC0,0x7F80,0xBF41,0x7D00,0xBDC1,0xBC81,0x7C40,
        0xB401,0x74C0,0x7580,0xB541,0x7700,0xB7C1,0xB681,0x7640,
        0x7200,0xB2C1,0xB381,0x7340,0xB101,0x71C0,0x7080,0xB041,
        0x5000,0x90C1,0x9181,0x5140,0x9301,0x53C0,0x5280,0x9241,
        0x9601,0x56C0,0x5780,0x9741,0x5500,0x95C1,0x9481,0x5440,
        0x9C01,0x5CC0,0x5D80,0x9D41,0x5F00,0x9FC1,0x9E81,0x5E40,
        0x5A00,0x9AC1,0x9B81,0x5B40,0x9901,0x59C0,0x5880,0x9841,
        0x8801,0x48C0,0x4980,0x8941,0x4B00,0x8BC1,0x8A81,0x4A40,
        0x4E00,0x8EC1,0x8F81,0x4F40,0x8D01,0x4DC0,0x4C80,0x8C41,
        0x4400,0x84C1,0x8581,0x4540,0x8701,0x47C0,0x4680,0x8641,
        0x8201,0x42C0,0x4380,0x8341,0x4100,0x81C1,0x8081,0x4040
};
#endif
extern const Uint16 crctable[];

#endif // MODBUSCOMDEF_H
