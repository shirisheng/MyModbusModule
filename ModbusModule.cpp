#include "ModbusModule.h"
//#include "debug.h"
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
    heartTimer_(0),
    reSendCount_(0),
    mudbusTimer_(0)
{
    this->runInfo_.status = IDLE_STATUS;
    this->runInfo_.exceptCode = 0;
    this->runInfo_.error = 0;
    this->runInfo_.errTimes = 0;
    this->packStuct_.buffFront = 0;
    this->packStuct_.buffRear = 0;
    this->unPackStuct_.recvLen = 0;
    this->unPackStuct_.dataLen = 0;
    this->unPackStuct_.pDataArea = NULL;
    this->pMudbusTimer_ = &mudbusTimer_;////////---
//    this->pMudbusTimer_ = initStruct.pMudbusTimer;
    this->timeOutTime_ = initStruct.timeOutTime;
    this->reSendTimes_ = initStruct.reSendTimes;
    this->packStuct_.pBuffLen = initStruct.packBuffLen;
    this->unPackStuct_.rBuffLen = initStruct.recvBuffLen;
    this->packStuct_.pPackBuff = new Uint8[initStruct.packBuffLen];
    this->unPackStuct_.pRecvBuff = new Uint8[initStruct.recvBuffLen];
    this->callBack_.pWriteData = initStruct.callBack.pWriteData;
    this->callBack_.pReadData = initStruct.callBack.pReadData;
    this->callBack_.pDataHandler = initStruct.callBack.pDataHandler;
    this->callBack_.pErrorHandler = initStruct.callBack.pErrorHandler;
    ///////////////////////////////////////////////////////////////---
    errorToDecrMap_.insert(Modbus_Error1, "打包缓冲区无效");
    errorToDecrMap_.insert(Modbus_Error2, "打包缓冲区空间不足");
    errorToDecrMap_.insert(Modbus_Error3, "接收缓冲区无效");
    errorToDecrMap_.insert(Modbus_Error4, "接收缓冲区空间不足");
    errorToDecrMap_.insert(Modbus_Error5, "定时器无效");
    errorToDecrMap_.insert(Modbus_Error6, "CRC校验错误");
    errorToDecrMap_.insert(Modbus_Error7, "从机返回异常响应帧");
    errorToDecrMap_.insert(Modbus_Error8,"通信超时");
    QObject::connect(&baseTimer_, SIGNAL(timeout()), this, SLOT(timeOutHandler()));
    baseTimer_.start(1);
    ///////////////////////////////////////////////////////////////---
}

ModbusModule::~ModbusModule()
{
    delete[] packStuct_.pPackBuff;
    delete[] unPackStuct_.pRecvBuff;
}

void ModbusModule::setSerialPort(SerialPortHelper* pSerialPort)
{
    pSerialPort_ = pSerialPort;
//    QObject::connect(pSerialPort_, SIGNAL(recvFinishSignal(Uint8*, Uint16)),
//                     this, SLOT(notifyModbusRecvFinish(Uint8*, Uint16)));
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

Uint8 ModbusModule::modbusInitCheck()
{
    if(this->packStuct_.pPackBuff == NULL)
    {
        this->runInfo_.error = Modbus_Error1;
        return 0;
    }
    if(this->unPackStuct_.pRecvBuff == NULL)
    {
        this->runInfo_.error = Modbus_Error3;
        return 0;
    }
    if(this->pMudbusTimer_ == NULL)
    {
        this->runInfo_.error = Modbus_Error5;
        return 0;
    }
    return 1;
}

Uint8 ModbusModule::insertElement(Uint8* pBuff, Uint16* pFront, Uint16* pRear, Uint8 data)
{
    if((*pRear + 1) % this->packStuct_.pBuffLen == *pFront)
        return 0; //队列已满时,不执行入队操作
    pBuff[*pRear] = data;
    /*尾指针指向下一个位置*/
    *pRear = (*pRear + 1) % this->packStuct_.pBuffLen;
    return 1;
}

Uint8 ModbusModule::deleteElement(Uint8* pBuff, Uint16* pFront, Uint16* pRear, Uint8* pData)
{
    if (*pFront == *pRear)
        return 0; //缓冲空队列，直接返回
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
    if(pBuff == NULL)
        return 0;
    /*插入包长度*/
    if(!insertElement(pBuff,&front,&rear,dataLen >> 8))
        return 0;
    /*插入包长度*/
    if(!insertElement(pBuff,&front,&rear,dataLen & 0x0FF))
        return 0;
    /*插入包内容*/
    for(i = 0; i < dataLen; i++)
        if(!insertElement(pBuff,&front,&rear,pPackData[i]))
            return 0;
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
    if(!deleteElement(pBuff,&front,&rear,&lenTmp))
        return 0;
    *dataLen = (lenTmp << 8);
    if(!deleteElement(pBuff,&front,&rear,&lenTmp))
        return 0;
    *dataLen |= (lenTmp & 0x0FF);
    for(i = 0; i < *dataLen; i++)
        if(!deleteElement(pBuff,&front,&rear,pPackData + i))
            return 0;
    return 1;
}

Uint8 ModbusModule::deleteCurrPack(Uint8* pBuff, Uint16* pFront, Uint16* pRear)
{
    Uint16 i = 0;
    Uint8 valTmp;
    Uint16 packLen;
    Uint16 front = *pFront;
    Uint16 rear = *pRear;
    if(!deleteElement(pBuff,&front,&rear,&valTmp))
        return 0;
    packLen = (valTmp << 8);
    if(!deleteElement(pBuff,&front,&rear,&valTmp))
        return 0;
    packLen |= (valTmp & 0x0FF);
    for(i = 0; i < packLen; i++)
        if(!deleteElement(pBuff,&front,&rear,&valTmp))
            return 0;
    *pFront = front;
    return 1;
}


Uint8 ModbusModule::toFunCode03CmdPackRTU(Uint8 slaveID, Uint16 startReg,  Uint16 regNum)
{
    Uint8 ret;
    Uint16 crc16 ,packLen;
    ReadMulRegCMD_X03 *pFrame;
    Uint8 packBuff[sizeof(ReadMulRegCMD_X03)/sizeof(Uint8) + 2];
    pFrame = (ReadMulRegCMD_X03 *)packBuff;
    pFrame->slaveID = slaveID;
    pFrame->funCode = READ_MUL_HLD_REG;
    pFrame->startRegH = (startReg >> 8);
    pFrame->startRegL = startReg & 0x0FF;
    pFrame->regNumH = (regNum >> 8);
    pFrame->regNumL = regNum & 0x0FF;
    crc16 = createCRC16(packBuff, sizeof(ReadMulRegCMD_X03) - 2);
    pFrame->crcH = (crc16 >> 8);
    pFrame->crcL = crc16 & 0x0FF;
    packLen = sizeof(ReadMulRegCMD_X03);
    ret = insertPack(this->packStuct_.pPackBuff,
                           &this->packStuct_.buffFront,
                           &this->packStuct_.buffRear,
                           packBuff, packLen);
    if(ret == 0) this->runInfo_.error = Modbus_Error2;
    return ret;
}

Uint8 ModbusModule::toFunCode06CmdPackRTU(Uint8 slaveID, Uint16 startReg, Uint16 data)
{
    Uint8 ret;
    Uint16 crc16 ,packLen;
    WriteOneRegCMD_X06 *pFrame;
    Uint8 packBuff[sizeof(WriteOneRegCMD_X06)/sizeof(Uint8) + 2];
    pFrame = (WriteOneRegCMD_X06 *)packBuff;
    pFrame->slaveID = slaveID;
    pFrame->funCode = WRITE_ONE_HLD_REG;
    pFrame->startRegH = (startReg >> 8);
    pFrame->startRegL = startReg & 0x0FF;
    pFrame->dataH = (data >> 8);
    pFrame->dataL = data & 0x0FF;
    crc16 = createCRC16(packBuff, sizeof(WriteOneRegCMD_X06) - 2);
    pFrame->crcH = (crc16 >> 8);
    pFrame->crcL = crc16 & 0x0FF;
    packLen = sizeof(WriteOneRegCMD_X06);
    ret = insertPack(this->packStuct_.pPackBuff,
                           &this->packStuct_.buffFront,
                           &this->packStuct_.buffRear,
                           packBuff, packLen);
    if(ret == 0) this->runInfo_.error = Modbus_Error2;
    return ret;
}

void ModbusModule::exceptRspHander(Uint8* pRecvBuff)
{
    ExceptionRSP *pFrame;
    pFrame = (ExceptionRSP*)pRecvBuff;
    this->runInfo_.exceptCode = pFrame->except;
    this->runInfo_.error = Modbus_Error7;
}

Uint8 ModbusModule::readDataFromSlave(Uint8 slaveID, Uint16 startAddr, Uint16 dataNum)
{
    if(this->runInfo_.status == ERROR_STATUS
    || this->runInfo_.error != Modbus_Error0)
        return 0;
    return toFunCode03CmdPackRTU(slaveID, startAddr, dataNum);
}

Uint8 ModbusModule::writeDataToSlave(Uint8 slaveID, Uint16 addr, Uint16 data)
{
    if(this->runInfo_.status == ERROR_STATUS
    || this->runInfo_.error != Modbus_Error0)
        return 0;
    return toFunCode06CmdPackRTU(slaveID, addr, data);
}

void ModbusModule::timeOutHandler()
{
    this->mudbusTimer_++;
}

Uint8 ModbusModule::addDataToRecvBuff(Uint8* pBuff, Uint16 len)
{
    Uint16 i = 0, recvLenTmp;
    recvLenTmp = this->unPackStuct_.recvLen;
    if(recvLenTmp + len > this->unPackStuct_.rBuffLen)
    {
        this->runInfo_.error = Modbus_Error4;
        return 0;
    }
    for(i = 0; i < len; i++)
    {
        this->unPackStuct_.pRecvBuff[recvLenTmp + i] = pBuff[i];
    }
    this->unPackStuct_.recvLen += len;
    return 1;
}

Uint8 ModbusModule::sendCurrCmdPackRTU()
{
    Uint16 dataLen = 0;
    Uint8 retVal, pPackData[50]; //数据包发送临时缓冲区
    static Uint16 bytesWr = 0;
    retVal = getCurrPack(this->packStuct_.pPackBuff,
                       &(this->packStuct_.buffFront),
                       &(this->packStuct_.buffRear),
                       pPackData, &dataLen);
    if(retVal == 1)
    {
        bytesWr += this->callBack_.
                pWriteData(pPackData + bytesWr, dataLen);
        if(bytesWr >= dataLen)
        {
            retVal = 2; //发送完成
            bytesWr = 0;
            this->unPackStuct_.recvLen = 0;
        }
    }
    if(retVal == 2)
    {
        pSerialPort_->showInCommBrowser(QString("Error Times: "),
                      QString::number(this->runInfo_.errTimes));
        pSerialPort_->showInCommBrowser(QString("发送内容:"),
                      QByteArray((char *)pPackData, dataLen));
//        debug("Error Times: %d",this->runInfo_.errTimes);
//        debug("发送内容:");
//        int i = 0;
//        for(i = 0; i < dataLen; i++)
//        {
//            debug("%d ",pPackData[i]);
//        }
//        debug("\n\n\n");
    }
    return retVal;
}

Uint8 ModbusModule::getCmdPackHeader(Uint8* pHeader)
{
    Uint16 front = this->packStuct_.buffFront;
    if(front == this->packStuct_.buffRear)
        return 0;
    pHeader[0] = this->packStuct_.pPackBuff[front + 2];
    pHeader[1] = this->packStuct_.pPackBuff[front + 3];
    return 1;
}

Uint8 ModbusModule::getRspPackHeader()
{
    Uint8 retVal = 0, cmdPackHeader[2];
    Uint16 i,recvLen = this->unPackStuct_.recvLen;
    Uint8* pRecvBuff = this->unPackStuct_.pRecvBuff;
    if(!getCmdPackHeader(cmdPackHeader))
        return 0;
    for(i = 0; i < recvLen - 1; i++)
    {
        if(cmdPackHeader[0] == pRecvBuff[i] &&
           cmdPackHeader[1] == (pRecvBuff[i+1] & 0x7F))
        {
            retVal = 1;
            break;
        }
    }
    if(i != 0) //去掉无效数据，将包头对齐到RecvBuff首部
    {
        this->unPackStuct_.recvLen = 0;
        this->addDataToRecvBuff(pRecvBuff+i,recvLen-i);
    }
    return retVal;
}

Uint8 ModbusModule::getRspPackLen(Uint8* rspPackLen)
{
    Uint8 dataLen = 0,retVal = 0;
    Uint8* pRecvBuff = this->unPackStuct_.pRecvBuff;
    switch(pRecvBuff[1])
    {
    case READ_MUL_HLD_REG:
        if(this->unPackStuct_.recvLen >= 3)
        {
            retVal = 1;dataLen = pRecvBuff[2];
            *rspPackLen = 3 + dataLen + 2;
        }
        break;
    case WRITE_ONE_HLD_REG:
        retVal = 1;
        *rspPackLen = sizeof(WriteOneRegRSP_X06)
                /sizeof(Uint8);
        break;
    default:
        if(pRecvBuff[1] >= 0x80)
        { //从机返回异常功能码
            retVal = 1;
            *rspPackLen = sizeof(ExceptionRSP)
                    /sizeof(Uint8);
        }
        break;
    }
    return retVal;
}

Uint8 ModbusModule::receiveRspPackRTU()
{
    Sint16 bytes = 0;
    Uint8 recvOk = 0,tmpBuff[10];
    static Uint8 rspPackLen = 0;
    static Uint8 recvStep = 1;
    Uint16 recvLen = this->unPackStuct_.recvLen;
    bytes = this->callBack_.pReadData(tmpBuff, 8);
    if(bytes > 0)
        this->addDataToRecvBuff(tmpBuff, bytes);
    if(*(this->pMudbusTimer_) > this->timeOutTime_)
        {recvStep = 1; this->runInfo_.error = Modbus_Error8;}
    switch(recvStep)
    {
    case 1:
        if(getRspPackHeader())
            recvStep = 2;
        if(recvStep == 1)
            break;
    case 2:
        if(getRspPackLen(&rspPackLen))
            recvStep = 3;
        if(recvStep == 2)
            break;
    case 3:
        if(recvLen >= rspPackLen)
        {
            recvStep = 1;
            recvOk = 1;
        }
        break;
    default:
        recvStep = 1;
        break;
    }
    if(recvOk == 1)
    {
        pSerialPort_->showInCommBrowser(QString("接收内容:"),
                      QByteArray((char *)this->unPackStuct_.pRecvBuff, this->unPackStuct_.recvLen));
//        debug("接收内容:");
//        int i = 0;
//        for(i = 0; i < this->unPackStuct_.recvLen; i++)
//        {
//            debug("%d ",this->unPackStuct_.pRecvBuff[i]);
//        }
//        debug("\n\n\n");
    }
    return recvOk;
}

Uint8 ModbusModule::handleRspPackRTU()
{
    Uint16 crc16, crc16Recv;
    Uint16 i = 0, rspDataLen = 0;
    Uint16 dataTmp, *pDataAreaTmp = NULL;
    Uint16 rspPackLen = this->unPackStuct_.recvLen;
    Uint8* pRecvBuff = this->unPackStuct_.pRecvBuff;
    this->unPackStuct_.dataLen = 0;
    crc16 = createCRC16(pRecvBuff, rspPackLen - 2);
    crc16Recv = (pRecvBuff[rspPackLen - 1] << 8)
              | (pRecvBuff[rspPackLen - 2] & 0x0FF);
    if(crc16 != crc16Recv)
    {
        this->runInfo_.error = Modbus_Error6;
        return 0;
    }
    if(pRecvBuff[1] >= 0x80)
    { //从机返回异常功能码
        exceptRspHander(pRecvBuff);
        return 0;
    }
    switch(pRecvBuff[1])
    {
    case READ_MUL_HLD_REG:
        pDataAreaTmp = (Uint16*)(pRecvBuff + 3);
        rspDataLen = pRecvBuff[2];
        for(i = 0; i < rspDataLen/2; i++)
        {
#if   (TRF_ORDER == 0)
            dataTmp = (pRecvBuff[3+i*2] << 8);
            dataTmp |= pRecvBuff[3+i*2+1];
#elif (TRF_ORDER == 1)
            dataTmp = (pRecvBuff[3+i*2+1] << 8);
            dataTmp |= pRecvBuff[3+i*2];
#endif
            pDataAreaTmp[i] = dataTmp;
        }
        this->unPackStuct_.dataLen = rspDataLen/2;
        this->unPackStuct_.pDataArea = pDataAreaTmp;
        break;
    case WRITE_ONE_HLD_REG:
        break;
    default:break;
    }
    if(this->unPackStuct_.dataLen > 0)
    {
        pSerialPort_->showInCommBrowser(QString("解析结果:"),
                      QByteArray((char *)this->unPackStuct_.pDataArea, this->unPackStuct_.dataLen*2));
//        debug("解析结果:");
//        int i = 0;
//        for(i = 0; i < this->unPackStuct_.dataLen; i++)
//        {
//            debug("%d ",this->unPackStuct_.pDataArea[i]);
//        }
//        debug("\n\n\n");
        this->callBack_.pDataHandler(this->unPackStuct_.pDataArea,
                                     this->unPackStuct_.dataLen);
    }
    return 1;
}

void ModbusModule::clearModbusError()
{
    if(this->runInfo_.status == RECV_FINISH
    || this->runInfo_.status == ERROR_STATUS)
    {
        this->runInfo_.status = IDLE_STATUS;
        this->reSendCount_ = 0;
        this->runInfo_.error = 0;
        this->runInfo_.exceptCode = 0;
        deleteCurrPack(this->packStuct_.pPackBuff, //当前帧出队，继续发下一帧
                               &this->packStuct_.buffFront,
                               &this->packStuct_.buffRear);
    }
}

void ModbusModule::heartbeatDetect()
{
    if(heartTimer_++ > 10000)
    {
        heartTimer_ = 0;
        this->readDataFromSlave(1, 0x01, 1);//心跳包
//        this->readDataFromSlave(1, 0x0300C, 1);//心跳包
    }
}

void ModbusModule::modbusErrorHandler()
{
    if(this->runInfo_.status == ERROR_STATUS
    || this->runInfo_.error == Modbus_Error0)
        return;
    this->runInfo_.errTimes++;
    if(this->runInfo_.error == Modbus_Error2
    || this->runInfo_.error == Modbus_Error6)
    {
        if(this->reSendCount_++ < this->reSendTimes_)
        {
            this->runInfo_.error = Modbus_Error0;
            this->runInfo_.status = SEND_STATUS;
        }
        else
            this->runInfo_.status = ERROR_STATUS;
    }
    else
        this->runInfo_.status = ERROR_STATUS;
    if(this->runInfo_.error != Modbus_Error0)
    {
        pSerialPort_->showInCommBrowser(QString("Error: ").append(QString::number(this->runInfo_.error)),
                      QString(errorToDecrMap_.value(this->runInfo_.error)));
//        debug("Error: %d", this->runInfo_.error);
//        debug("\n\n\n");
    }
}

void ModbusModule::runningModbus()
{
    Uint8 ret = 0;
    Uint16 front, rear;
    modbusInitCheck();
    modbusErrorHandler();
    front = this->packStuct_.buffFront;
    rear = this->packStuct_.buffRear;
    switch (this->runInfo_.status)
    {
    case IDLE_STATUS:
        if(front != rear)
            this->runInfo_.status = SEND_STATUS;
        else //心跳检测
        {
            this->heartbeatDetect();
        }
        break;
    case SEND_STATUS:
        ret = sendCurrCmdPackRTU();
        if(ret == 0)
            this->runInfo_.status = IDLE_STATUS;
        else if(ret == 1)
            this->runInfo_.status = SEND_STATUS;
        else if(ret == 2)
        {
            *(this->pMudbusTimer_ ) = 0;
            this->runInfo_.status = RECV_STATUS;
        }
        break;
    case RECV_STATUS:
        if(receiveRspPackRTU())
            this->runInfo_.status = RECV_FINISH;
        break;
    case RECV_FINISH:
        if(handleRspPackRTU())
        {
            clearModbusError();
        }
        break;
    case ERROR_STATUS:
        if(this->runInfo_.error != Modbus_Error0)
            this->callBack_.pErrorHandler();
        /*清除错误，防止pCommErrorHandler反复调用*/
        this->runInfo_.error = Modbus_Error0;
        break;
    default:
        this->runInfo_.status = IDLE_STATUS;
        break;
    }
}
