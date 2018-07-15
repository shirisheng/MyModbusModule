#include "ModbusModule.h"
#include "SerialPortHelper.h"

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

ModbusModule::ModbusModule(ModbusInitStruct initStruct):
    reSendTimes_(10),
    reSendCount_(0),
    mudbusTimer_(0)
{
    this->runInfo_.status = IDLE_STATUS;
    this->packStuct_.buffFront = 0;
    this->packStuct_.buffRear = 0;
    this->unPackStuct_.dataLen = 0;
    this->timeOutTime_ = initStruct.timeOutTime;
    this->clearModbusError();
    this->packStuct_.pPackBuff = new Uint8[initStruct.packBuffLen];
    this->packStuct_.pBuffLen = initStruct.packBuffLen;
    this->unPackStuct_.pRecvBuff = initStruct.pRecvBuff;
    this->unPackStuct_.rBuffLen = initStruct.recvBuffLen;
    this->unPackStuct_.pDataBuff = new Uint16[initStruct.dataBuffLen];
    this->callBackStruct_.pDataSender = initStruct.callBack.pDataSender;
    this->callBackStruct_.pDataHandler = initStruct.callBack.pDataHandler;
    this->callBackStruct_.pCommErrorHandler = initStruct.callBack.pCommErrorHandler;
    errorToDecrMap_.insert(Modbus_Error1, "从机地址错误");
    errorToDecrMap_.insert(Modbus_Error2, "功能码不匹配");
    errorToDecrMap_.insert(Modbus_Error3, "CRC校验错误");
    errorToDecrMap_.insert(Modbus_Error4, "解包缓冲区无效");
    errorToDecrMap_.insert(Modbus_Error5, "解包缓冲区空间不足");
    errorToDecrMap_.insert(Modbus_Error6, "打包缓冲区无效");
    errorToDecrMap_.insert(Modbus_Error7, "打包缓冲区空间不足");
    errorToDecrMap_.insert(Modbus_Error8, "接收缓冲区无效");
    errorToDecrMap_.insert(Modbus_Error9, "接收缓冲区空间不足");
    errorToDecrMap_.insert(Modbus_Error10,"从机返回异常响应帧");
    errorToDecrMap_.insert(Modbus_Error11,"功能码未实现");
    errorToDecrMap_.insert(Modbus_Error12,"通信超时");
}

ModbusModule::~ModbusModule()
{
    delete[] packStuct_.pPackBuff;
    delete[] unPackStuct_.pDataBuff;
}

void ModbusModule::setSerialPort(SerialPortHelper* pSerialPort)
{
    pSerialPort_ = pSerialPort;
    QObject::connect(pSerialPort_, SIGNAL(recvFinishSignal(Uint8*, Uint16)),
                     this, SLOT(notifyModbusRecvFinish(Uint8*, Uint16)));
}

Uint16 ModbusModule::createCRC16(Uint8 *str,Uint16 num)
{
    Uint8 i,arc;
    Uint16 crc;
    crc = 0xffff;

    const Uint16 *crctable_temp = crctable;
    for (i = 0; i < num; ++i)
    {
        arc = (str[i] ^ crc) & 0x00ff;
        crc = crc >> 8;
        crc = crc & 0x00ff;
        crc = crc ^ crctable_temp[arc];
    }
    return(crc);
}

Uint8 ModbusModule::insertElement(Uint8* pBuff, Uint16* pFront, Uint16* pRear, Uint8 data)
{
    if((*pRear + 1) % this->packStuct_.pBuffLen == *pFront)
        return 0; //队列已满时,不执行入队操作
    pBuff[*pRear] = data;
    *pRear = (*pRear + 1) % this->packStuct_.pBuffLen; //尾部指向下一个位置
    return 1;
}

Uint8 ModbusModule::deleteElement(Uint8* pBuff, Uint16* pFront, Uint16* pRear, Uint8* pData)
{
    if (*pFront == *pRear)  return 0; //缓冲空队列，直接返回
    *pData = pBuff[*pFront];
    *pFront = (*pFront + 1) % this->packStuct_.pBuffLen;
     return 1;
}

Uint8 ModbusModule::insertPack(Uint8* pBuff, Uint16* pFront, Uint16* pRear,
                               Uint8* pPackData, Uint16 dataLen)
{
    Uint16 i = 0;
    Uint16 front = *pFront;
    Uint16 rear = *pRear;
    if(pBuff == NULL) return 0;
    if(!insertElement(pBuff,&front,&rear,dataLen >> 8)) return 0;     //插入包长度
    if(!insertElement(pBuff,&front,&rear,dataLen & 0x0FF)) return 0;  //插入包长度
    for(i = 0; i < dataLen; i++)
        if(!insertElement(pBuff,&front,&rear,pPackData[i])) return 0; //插入包内容
    *pRear = rear;
    return 1;
}

Uint8 ModbusModule::getCurrPack(Uint8* pBuff, Uint16* pFront, Uint16* pRear,
                                Uint8* pPackData, Uint16* dataLen)
{
    Uint16 i = 0;
    Uint8 lenTmp;
    Uint16 front = *pFront;
    Uint16 rear = *pRear;
    if(!deleteElement(pBuff,&front,&rear,&lenTmp)) return 0;
    *dataLen = (lenTmp << 8);
    if(!deleteElement(pBuff,&front,&rear,&lenTmp)) return 0;
    *dataLen |= (lenTmp & 0x0FF);
    for(i = 0; i < *dataLen; i++)
        if(!deleteElement(pBuff,&front,&rear,pPackData + i)) return 0;
    return 1;
}

Uint8 ModbusModule::deleteCurrPack(Uint8* pBuff, Uint16* pFront, Uint16* pRear)
{
    Uint16 i = 0;
    Uint8 valTmp;
    Uint16 packLen;
    Uint16 front = *pFront;
    Uint16 rear = *pRear;
    if(!deleteElement(pBuff,&front,&rear,&valTmp)) return 0;
    packLen = (valTmp << 8);
    if(!deleteElement(pBuff,&front,&rear,&valTmp)) return 0;
    packLen |= (valTmp & 0x0FF);
    for(i = 0; i < packLen; i++)
        if(!deleteElement(pBuff,&front,&rear,&valTmp)) return 0;
    *pFront = front;
    return 1;
}


bool ModbusModule::toFunCode03CmdPackRTU(Uint8 slaveID, Uint16 startReg,  Uint16 regNum)
{
    Uint8 ret;
    Uint16 crc16 ,packLen;
    ReadMulRegCMD_X03 *pFrame;
    Uint8 packBuff[sizeof(ReadMulRegCMD_X03)/sizeof(Uint8) + 2];
    pFrame = (ReadMulRegCMD_X03 *)packBuff;
    pFrame->slaveID = slaveID;
    pFrame->funCode = READ_MUL_HLD_REG;
    pFrame->startRegH = (startReg << 8);
    pFrame->startRegL = startReg & 0x0FF;
    pFrame->regNumH = (regNum << 8);
    pFrame->regNumL = regNum & 0x0FF;
    crc16 = createCRC16(packBuff, sizeof(ReadMulRegCMD_X03) - 2);
    pFrame->crcH = (crc16 >> 8);
    pFrame->crcL = crc16 & 0x0FF;
    packLen = sizeof(ReadMulRegCMD_X03);
    ret = insertPack(this->packStuct_.pPackBuff,
                           &this->packStuct_.buffFront,
                           &this->packStuct_.buffRear,
                           packBuff, packLen);
    if(ret == 0)
    {
        this->runInfo_.error = Modbus_Error7;
        return false;
    }
    return true;
}

bool ModbusModule::funCode03RspUnPackRTU()
{
    Uint16 i = 0,lenTmp;
    Uint16 crc16,crc16Recv;
    Uint16 rspDataLen;
    Uint16 rspPackLen;
    Uint8 rspHeaderLen = 3;
    Uint8 currCmdPack[10];
    Uint8* pUPkSrcBuff = this->unPackStuct_.pRecvBuff;
    Uint16* pUnPkData = this->unPackStuct_.pDataBuff;
    getCurrPack(this->packStuct_.pPackBuff,
                        &(this->packStuct_.buffFront),
                        &(this->packStuct_.buffRear),
                        currCmdPack, &lenTmp);
    ReadMulRegRSP_X03 *pRspFrame = (ReadMulRegRSP_X03 *)pUPkSrcBuff;
    ReadMulRegCMD_X03 *pCmdFrame = (ReadMulRegCMD_X03 *)currCmdPack;
    if (pUPkSrcBuff == NULL)
    {
        this->runInfo_.error = Modbus_Error8;
        return false;
    }
    if (pUnPkData == NULL)
    {
        this->runInfo_.error = Modbus_Error4;
        return false;
    }
    if(pRspFrame->slaveID != pCmdFrame->slaveID)
    {
        this->runInfo_.error = Modbus_Error1;
        return false;
    }
    if(pRspFrame->funCode != pCmdFrame->funCode)
    {
        this->runInfo_.error = Modbus_Error2;
        return false;
    }
    rspDataLen = pRspFrame->dataLen;
    if(this->unPackStuct_.dBuffLen < rspDataLen)
    {
        this->runInfo_.error = Modbus_Error5;
        return false;
    }
    rspPackLen = rspHeaderLen + rspDataLen + 2;
    crc16 = createCRC16(pUPkSrcBuff, rspPackLen - 2);
    crc16Recv = (pUPkSrcBuff[rspPackLen - 1] << 8) |
            (pUPkSrcBuff[rspPackLen - 2] & 0x0FF);
    if(crc16 != crc16Recv)
    {
        this->runInfo_.error = Modbus_Error3;
        return false;
    }
    for(i = 0; i < rspDataLen; i++)
    {
#if defined(HIGH_FIRST)
        pUnPkData[i] = (pUPkSrcBuff[rspHeaderLen + i*2] << 8) |
                pUPkSrcBuff[rspHeaderLen + i*2 + 1];
#elif defined(LOW_FIRST)
        pUnPkData[i] = (pUPkSrcBuff[rspHeaderLen + i*2 + 1] << 8) |
                pUPkSrcBuff[rspHeaderLen + i*2];
#endif
    }
    this->unPackStuct_.dataLen = rspDataLen;
    return true;
}

bool ModbusModule::toFunCode06CmdPackRTU(Uint8 slaveID, Uint16 startReg, Uint16 data)
{
    Uint8 ret;
    Uint16 crc16 ,packLen;
    WriteMulRegCMD_X06 *pFrame;
    Uint8 packBuff[sizeof(WriteMulRegCMD_X06)/sizeof(Uint8) + 2];
    pFrame = (WriteMulRegCMD_X06 *)packBuff;
    pFrame->slaveID = slaveID;
    pFrame->funCode = WRITE_ONE_HLD_REG;
    pFrame->startRegH = (startReg << 8);
    pFrame->startRegL = startReg & 0x0FF;
    pFrame->dataH = (data >> 8);
    pFrame->dataL = data & 0x0FF;
    crc16 = createCRC16(packBuff, sizeof(WriteMulRegCMD_X06) - 2);
    pFrame->crcH = (crc16 >> 8);
    pFrame->crcL = crc16 & 0x0FF;
    packLen = sizeof(WriteMulRegCMD_X06);
    ret = insertPack(this->packStuct_.pPackBuff,
                           &this->packStuct_.buffFront,
                           &this->packStuct_.buffRear,
                           packBuff, packLen);
    if(ret == 0)
    {
        this->runInfo_.error = Modbus_Error7;
        return false;
    }
    return true;
}

bool ModbusModule::funCode06RspUnPackRTU()
{
    Uint16 crc16, lenTmp;
    Uint8 currCmdPack[10];
    Uint8* pUPkSrcBuff = this->unPackStuct_.pRecvBuff;
    getCurrPack(this->packStuct_.pPackBuff,
                        &(this->packStuct_.buffFront),
                        &(this->packStuct_.buffRear),
                        currCmdPack, &lenTmp);
    WriteMulRegRSP_X06 *pRspFrame = (WriteMulRegRSP_X06 *)pUPkSrcBuff;
    WriteMulRegCMD_X06 *pCmdFrame = (WriteMulRegCMD_X06 *)currCmdPack;
    if (pUPkSrcBuff == NULL)
    {
        this->runInfo_.error = Modbus_Error8;
        return false;
    }
    if(pRspFrame->slaveID != pCmdFrame->slaveID)
    {
        this->runInfo_.error = Modbus_Error1;
        return false;
    }
    if(pRspFrame->funCode != pCmdFrame->funCode)
    {
        this->runInfo_.error = Modbus_Error2;
        return false;
    }
    crc16 = createCRC16(pUPkSrcBuff, sizeof(WriteMulRegRSP_X06) - 2);
    if(crc16 != (pRspFrame->crcH << 8 | pRspFrame->crcL))
    {
        this->runInfo_.error = Modbus_Error3;
        return false;
    }
    this->unPackStuct_.dataLen = 0;
    return true;
}

void ModbusModule::exceptRspHander(Uint8* pUPkSrcBuff)
{
    ExceptionRSP *pFrame;
    pFrame = (ExceptionRSP*)pUPkSrcBuff;
    this->runInfo_.exceptCode = pFrame->except;
}

bool ModbusModule::modbusRspUnPackRTU(Uint8* pUPkSrcBuff)
{
    if (pUPkSrcBuff == NULL)
    {
        this->runInfo_.error = Modbus_Error8;
        return false;
    }
    if(pUPkSrcBuff[1] >= 0x80)
    { //从机返回异常功能码
        exceptRspHander(pUPkSrcBuff);
        this->runInfo_.error = Modbus_Error10;
        return false;
    }
    else
    {
        this->runInfo_.exceptCode = 0;
    }
    switch(pUPkSrcBuff[1])
    {
    case READ_MUL_HLD_REG:
        return funCode03RspUnPackRTU();
    case WRITE_ONE_HLD_REG:
        return funCode06RspUnPackRTU();
//    case WRITE_MUL_HLD_REG:
//        return;
    default: //未实现的正常功能码
        this->runInfo_.error = Modbus_Error11;
        return false;
    }
}

bool ModbusModule::readDataFromSlave(Uint8 slaveID, Uint16 startAddr, Uint16 dataNum)
{
    if(this->runInfo_.status == ERROR_STATUS ||
       this->runInfo_.error != Modbus_Error0) return false;
    return toFunCode03CmdPackRTU(slaveID, startAddr, dataNum);
}

bool ModbusModule::writeDataToSlave(Uint8 slaveID, Uint16 startAddr, Uint16 data)
{
    if(this->runInfo_.status == ERROR_STATUS ||
       this->runInfo_.error != Modbus_Error0) return false;
    return toFunCode06CmdPackRTU(slaveID, startAddr, data);
}

void ModbusModule::notifyModbusRecvFinish(Uint8* pRecvBuff, Uint16 buffLen)
{
    Uint16 i = 0;
    if(buffLen < this->unPackStuct_.rBuffLen)
    {
        for(i = 0; i < buffLen; i++)
            this->unPackStuct_.pRecvBuff[i] = pRecvBuff[i];
        this->runInfo_.status = RECV_FINISH;
    }
    else
    {
        this->runInfo_.error = Modbus_Error9;
    }
}

void ModbusModule::clearErrAndSendNextPack()
{
    if(this->runInfo_.status == ERROR_STATUS)
    {
        this->runInfo_.status = IDLE_STATUS;
        this->mudbusTimer_ = 0;
        this->reSendCount_ = 0;
        this->runInfo_.error = 0;
        this->runInfo_.exceptCode = 0;
        deleteCurrPack(this->packStuct_.pPackBuff, //当前帧出队，继续发下一帧
                               &this->packStuct_.buffFront,
                               &this->packStuct_.buffRear);
    }
}

void ModbusModule::clearModbusError()
{
    this->mudbusTimer_ = 0;
    this->reSendCount_ = 0;
    this->runInfo_.error = 0;
    this->runInfo_.exceptCode = 0;
}

void ModbusModule::modbusErrorHandler()
{
    if(this->runInfo_.status == ERROR_STATUS ||
       this->runInfo_.error == Modbus_Error0) return;
    if(this->runInfo_.error == Modbus_Error1 ||
       this->runInfo_.error == Modbus_Error2 ||
       this->runInfo_.error == Modbus_Error3 ||
       this->runInfo_.error == Modbus_Error7 ||
       this->runInfo_.error == Modbus_Error12)
    { //
        if(this->reSendCount_++ <= this->reSendTimes_)
            this->runInfo_.status = SEND_STATUS;
        else this->runInfo_.status = ERROR_STATUS;
    }
    else this->runInfo_.status = ERROR_STATUS;
    pSerialPort_->showInCommBrowser(QString("Error:"),
                  QString(errorToDecrMap_.value(this->runInfo_.error)));
}

void ModbusModule::modbusRunningController()
{
    Uint16 dataLen, front, rear;
    Uint8 ret, pPackData[50]; //数据包发送临时缓冲区
    front = this->packStuct_.buffFront;
    rear = this->packStuct_.buffRear;
    if(this->runInfo_.status == ERROR_STATUS)
        return;
    modbusErrorHandler();
    if(this->runInfo_.status != RECV_STATUS)
        this->mudbusTimer_ = 0; // ms/1
    switch (this->runInfo_.status)
    {
    case IDLE_STATUS:
        if(front != rear)
            this->runInfo_.status = SEND_STATUS;
        break;
    case SEND_STATUS:
            ret = getCurrPack(this->packStuct_.pPackBuff,
                                      &front, &rear, pPackData, &dataLen);
            if(ret == 1)
            {
                this->callBackStruct_.pDataSender(pPackData, dataLen);
                this->runInfo_.status = RECV_STATUS;
            }
            else this->runInfo_.status = IDLE_STATUS;
        break;
    case RECV_STATUS:
        if(this->mudbusTimer_++ > timeOutTime_) //通信超时
        { //超时重发
            this->runInfo_.error = Modbus_Error12;
        }
        break;
    case RECV_FINISH:
        if(modbusRspUnPackRTU(this->unPackStuct_.pRecvBuff))
        {   //解包成功
            clearModbusError();
            this->runInfo_.status = IDLE_STATUS;
            deleteCurrPack(this->packStuct_.pPackBuff, &front, &rear);
            this->packStuct_.buffFront = front; //当前帧出队，继续发下一帧
            pSerialPort_->showInCommBrowser(QString("解析结果:"),
                          QByteArray((char *)this->unPackStuct_.pDataBuff, this->unPackStuct_.dataLen));
            this->callBackStruct_.pDataHandler(this->unPackStuct_.pDataBuff,
                                                     this->unPackStuct_.dataLen);
        }
        break;
    case ERROR_STATUS:
        if(this->runInfo_.error != Modbus_Error0)
            this->callBackStruct_.pCommErrorHandler();
        clearModbusError();
        break;
    default:
        this->runInfo_.status = IDLE_STATUS;
        break;
    }
}

