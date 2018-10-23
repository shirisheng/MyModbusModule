#include "ModbusSlave.h"

ModbusSlave::ModbusSlave(SlaveInitStruct initStruct):
    rGapTime_(0),
    fGapTime_(0)
{
    slaveID_ = initStruct.slaveID;
    baudRate_ = initStruct.baudRate;
    baseTimer_.pTimeCounter = initStruct.pTimeCounter;
    baseTimer_.countCyclTime = initStruct.countCyclTime;
    recvBuff_.hasRecv = 0;
    recvBuff_.recvLen = initStruct.recvBuffLen;
    recvBuff_.buffLen = initStruct.recvBuffLen;
    recvBuff_.pBuff = new Uint8[initStruct.recvBuffLen];
    sendBuff_.hasSend = 0;
    sendBuff_.sendLen = 0;
    sendBuff_.buffLen = initStruct.recvBuffLen;
    sendBuff_.pBuff = new Uint8[initStruct.sendBuffLen];
#ifndef DEBUG_CODE
    callBack_.readRegister = initStruct.callBack.readRegister;
    callBack_.writeRegister = initStruct.callBack.writeRegister;
#endif
    callBack_.readComDev = initStruct.callBack.readComDev;
    callBack_.writeComDev = initStruct.callBack.writeComDev;
    callBack_.hasDataInComDev = initStruct.callBack.hasDataInComDev;
    runInfo_.errTimes = 0;
    runInfo_.error = SLAVE_Error0;
    runInfo_.status = SLAVE_IDLE_STATUS;
    createGapTime(baudRate_);

    if(sendBuff_.pBuff != NULL
	&&recvBuff_.pBuff != NULL
	&&callBack_.readComDev != NULL
    && callBack_.writeComDev != NULL
    && callBack_.hasDataInComDev != NULL
#ifndef DEBUG_CODE
    && baseTimer_.pTimeCounter != NULL
    && callBack_.readRegister != NULL
    && callBack_.writeRegister != NULL
#endif
            )
    {
        isInitOK_ = TRUE;
    } else isInitOK_ = FALSE;
#ifdef DEBUG_CODE
    timerCounter_ = 0;
    pSerialPort_ = NULL;
    for(int i = 0; i < 500; i++)
        memSpace[i] = i;
    errorToDecrMap_.insert(SLAVE_Error1, "初始化失败");
    errorToDecrMap_.insert(SLAVE_Error2, "发送缓冲区空间不足");
    errorToDecrMap_.insert(SLAVE_Error3, "接收缓冲区空间不足");
    errorToDecrMap_.insert(SLAVE_Error4, "读寄存器缓冲区不足");
    errorToDecrMap_.insert(SLAVE_Error5, "CRC校验错误");
    errorToDecrMap_.insert(SLAVE_Error6, "接收长度错误");
    QObject::connect(&debugTimer_, SIGNAL(timeout()), this, SLOT(timeOutHandler()));
    baseTimer_.countCyclTime = 1;
    baseTimer_.pTimeCounter = &timerCounter_;
    debugTimer_.start(1);
#endif
}

ModbusSlave::~ModbusSlave()
{
    delete[] recvBuff_.pBuff;
    delete[] sendBuff_.pBuff;
}

Uint16 ModbusSlave::createCRC16(Uint8 *str,Uint16 num)
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

void ModbusSlave::baseTimerHandler()
{
    Uint8 incrTime = (*baseTimer_.pTimeCounter)
            *baseTimer_.countCyclTime;
    rGapTime_ += incrTime;
    *(baseTimer_.pTimeCounter) = 0;
}

#ifdef DEBUG_CODE
void ModbusSlave::setSerialPort(SerialPortHelper* pSerialPort)
{
    pSerialPort_ = pSerialPort;
}

void ModbusSlave::timeOutHandler()
{
    this->timerCounter_++;
}
#endif

Sint16 ModbusSlave::readRegister(Uint16* buff, Uint16 addr, Uint16 num)
{
#ifdef DEBUG_CODE
    for(int i = 0; i < num; i++)
        buff[i] = memSpace[addr+i];
    return num;
#else
    return callBack_.readRegister(buff, addr , num);
#endif
}

Sint16 ModbusSlave::writeRegister(Uint16* buff, Uint16 addr, Uint16 num)
{
#ifdef DEBUG_CODE
    for(int i = 0; i < num; i++)
        memSpace[addr+i] = buff[i];
    return num;
#else
    return callBack_.writeRegister(buff, addr , num);
#endif
}

void ModbusSlave::createGapTime(Uint32 baudRate)
{
    /// @brief 传输一个字符包含10位的二进制数据（起始位1+数据位8+停止位1）
    this->fGapTime_ = (1.0/baudRate)*10*3.5*1000;
}

Bool ModbusSlave::isCmdPackComeIn()
{
    return callBack_.hasDataInComDev();
}

void ModbusSlave::prepareForRecv()
{
	rGapTime_ = 0;
    recvBuff_.hasRecv = 0;
}

/// @brief RTU模式的帧结束判断条件为: 3.5个字符时间内未接收到数据
Bool ModbusSlave::recvCmdPackRTU()
{
    Bool retVal = FALSE;
    Sint16 rdRetVal = 0;
    Uint8* buff = recvBuff_.pBuff;
    Uint16 recvLen = recvBuff_.recvLen;
    Uint16 hasRecv = recvBuff_.hasRecv;
    if(recvLen > recvBuff_.buffLen
    || hasRecv >= recvLen)
    {
        runInfo_.error = SLAVE_Error3;
        return FALSE;
    }
    if(this->isCmdPackComeIn())
    { //若通信设备接收缓冲区接收到数据
        rGapTime_ = 0;
        rdRetVal = callBack_.readComDev
                (buff + hasRecv, recvLen - hasRecv);
        recvBuff_.hasRecv +=
        		((rdRetVal > 0) ? rdRetVal : 0);
    }
    if(rGapTime_ > fGapTime_)
    { //若帧结束判断条件成立
        retVal = TRUE;
#ifdef DEBUG_CODE
        pSerialPort_->showInCommBrowser(QString("Error Times: "),
                      QString::number(this->runInfo_.errTimes));
        pSerialPort_->showInCommBrowser(QString("接收内容:"),
                      QByteArray((char *)(recvBuff_.pBuff),
                      recvBuff_.hasRecv));
#endif
    }
    return retVal;
}

void ModbusSlave::prepareForSend()
{
    sendBuff_.hasSend = 0;
}

Bool ModbusSlave::sendRspPackRTU()
{
    Bool retVal = FALSE;
    Sint16 wrRetVal = 0;
    Uint8* buff = sendBuff_.pBuff;
    Uint16 hasSend = sendBuff_.hasSend;
    Uint16 packLen = sendBuff_.sendLen;
    if(sendBuff_.hasSend < packLen)
    {
        wrRetVal = callBack_.writeComDev
        		(buff + hasSend, packLen - hasSend);
        sendBuff_.hasSend +=
        		((wrRetVal > 0) ? wrRetVal : 0);
    }
    else
    {
        retVal = TRUE; //发送完成
#ifdef DEBUG_CODE
        pSerialPort_->showInCommBrowser(QString("发送内容:"),
                      QByteArray((char *)(sendBuff_.pBuff),
                      sendBuff_.hasSend));
#endif
    }
    return retVal;
}

Bool ModbusSlave::isSlaveIDValid()
{
    return this->recvBuff_.pBuff[0] == slaveID_;
}

Bool ModbusSlave::isSendBuffEnough(Uint16 sendLen)
{
    Bool retVal = TRUE;
    if(sendLen > sendBuff_.buffLen)
    {
        sendBuff_.sendLen = 0;
        runInfo_.error = SLAVE_Error2;
        retVal = FALSE;
    }
    return retVal;
}

Bool ModbusSlave::funCode03CmdPackHandle()
{
    Uint16 tmpBuff[100];
    Uint16 i = 0, cmdPackLen = 0;
    Uint16 addr = 0, num = 0;
    Uint16 crc = 0, crcRecv = 0;
    ReadMulRegCMD_X03* cmdFrame = (ReadMulRegCMD_X03*)recvBuff_.pBuff;
    ReadMulRegRSP_X03* rspFrame = (ReadMulRegRSP_X03*)sendBuff_.pBuff;
    cmdPackLen = sizeof(ReadMulRegCMD_X03)/sizeof(Uint8);
    if(recvBuff_.hasRecv != cmdPackLen)
    {
        runInfo_.error = SLAVE_Error6;
        return FALSE;
    }
    addr = cmdFrame->startRegL + cmdFrame->startRegH*256;
    if(0)
    { //检查地址（addr和addr+num之间的地址）是否合法
        return createExpRspPack(Except_Code2);
    }
    num = cmdFrame->regNumL + cmdFrame->regNumH*256;
    if(num > 100)
    {
        runInfo_.error = SLAVE_Error4;
        return FALSE;
    }
    crcRecv =cmdFrame->crcL + cmdFrame->crcH*256;
    crc = createCRC16(recvBuff_.pBuff,
                      sizeof(ReadMulRegCMD_X03)/sizeof(Uint8) - 2);
    if(crc != crcRecv)
    {
        runInfo_.error = SLAVE_Error5;
        return FALSE;
    }
    sendBuff_.sendLen = 3 + num*2 + 2;
    if(!isSendBuffEnough(sendBuff_.sendLen))
        return FALSE;
    readRegister(tmpBuff, addr , num);
    rspFrame->slaveID = cmdFrame->slaveID;
    rspFrame->funCode = cmdFrame->funCode;
    rspFrame->dataLen = num*2;
    for(i = 0; i < num; i++)
    {
        sendBuff_.pBuff[3+i*2] = tmpBuff[i] & 0x0FF;
        sendBuff_.pBuff[3+i*2+1] = (tmpBuff[i] >> 8) & 0x0FF;
    }
    crc = createCRC16(sendBuff_.pBuff, sendBuff_.sendLen - 2);
    sendBuff_.pBuff[sendBuff_.sendLen - 2] = crc & 0x0FF;
    sendBuff_.pBuff[sendBuff_.sendLen - 1] = (crc >> 8) & 0x0FF;
    return TRUE;
}

Bool ModbusSlave::funCode06CmdPackHandle()
{
	Uint16 cmdPackLen = 0;
    Uint16 crc = 0, crcRecv = 0;
    Uint16 addr = 0, data = 0;
    WriteOneRegCMD_X06* cmdFrame = (WriteOneRegCMD_X06*)recvBuff_.pBuff;
    WriteOneRegRSP_X06* rspFrame = (WriteOneRegRSP_X06*)sendBuff_.pBuff;
    cmdPackLen = sizeof(WriteOneRegCMD_X06)/sizeof(Uint8);
    if(recvBuff_.hasRecv != cmdPackLen)
    {
        runInfo_.error = SLAVE_Error6;
        return FALSE;
    }
    addr = cmdFrame->startRegL + cmdFrame->startRegH*256;
    if(0)
    { //检查地址addr是否合法
        return createExpRspPack(Except_Code2);
    }
    data = cmdFrame->dataL + cmdFrame->dataH*256;
    if(0)
    { //检查要写入的数据data是否合法
        return createExpRspPack(Except_Code3);
    }
    crcRecv =cmdFrame->crcL + cmdFrame->crcH*256;
    crc = createCRC16(recvBuff_.pBuff,
                      sizeof(WriteOneRegRSP_X06)/sizeof(Uint8) - 2);
    if(crc != crcRecv)
    {
        runInfo_.error = SLAVE_Error5;
        return FALSE;
    }
    sendBuff_.sendLen = sizeof(WriteOneRegRSP_X06)/sizeof(Uint8);
    if(!isSendBuffEnough(sendBuff_.sendLen))
        return FALSE;
    writeRegister(&data, addr , 1);
    rspFrame->slaveID = cmdFrame->slaveID;
    rspFrame->funCode = cmdFrame->funCode;
    rspFrame->startRegH = cmdFrame->startRegH;
    rspFrame->startRegL = cmdFrame->startRegL;
    rspFrame->dataH = cmdFrame->dataH;
    rspFrame->dataL = cmdFrame->dataL;
    rspFrame->crcL = cmdFrame->crcL;
    rspFrame->crcH = cmdFrame->crcH;
    crc = createCRC16(sendBuff_.pBuff, sendBuff_.sendLen - 2);
    rspFrame->crcL = crc & 0x0FF;
    rspFrame->crcH = (crc >> 8) & 0x0FF;
    return TRUE;
}

Bool ModbusSlave::createExpRspPack(Uint8 exceptCode)
{
    Uint16 crc = 0;
    ExceptionRSP* rspFrame = (ExceptionRSP*)sendBuff_.pBuff;
    sendBuff_.sendLen = sizeof(ExceptionRSP)/sizeof(Uint8);
    if(!isSendBuffEnough(sendBuff_.sendLen))
        return FALSE;
    sendBuff_.pBuff[0] = recvBuff_.pBuff[0];
    sendBuff_.pBuff[1] = recvBuff_.pBuff[1] | 0x80;
    rspFrame->except = exceptCode;
    crc = createCRC16(sendBuff_.pBuff,sendBuff_.sendLen - 2);
    rspFrame->crcL = crc & 0x0FF;
    rspFrame->crcH = (crc >> 8) & 0x0FF;
    return TRUE;
}

Bool ModbusSlave::masterCmdPackHandle()
{
    if(!this->isSlaveIDValid())
    {
    	runInfo_.status = SLAVE_IDLE_STATUS;
        return FALSE;
    }
    switch(this->recvBuff_.pBuff[1])
    {
    case READ_MUL_HLD_REG:
        return this->funCode03CmdPackHandle();
    case WRITE_ONE_HLD_REG:
        return this->funCode06CmdPackHandle();
    default:
        return createExpRspPack(Except_Code1);
    }
}

void ModbusSlave::slaveErrorHandler()
{
    if(runInfo_.error != SLAVE_Error0)
    {
    	if(runInfo_.error != SLAVE_Error6)
    		runInfo_.errTimes++;
        runInfo_.status = SLAVE_ERROR_STATUS;
    }
#ifdef DEBUG_CODE
    if(runInfo_.error != SLAVE_Error0)
    {
        pSerialPort_->showInCommBrowser(QString("Error: ")
                     .append(QString::number(this->runInfo_.error)),
                      QString(errorToDecrMap_.value(this->runInfo_.error)));
    }
#endif
}

void ModbusSlave::clearSlaveError()
{
    if(this->runInfo_.status == SLAVE_ERROR_STATUS)
    {
        runInfo_.error = SLAVE_Error0;
        runInfo_.status = SLAVE_IDLE_STATUS;
    }
}

void ModbusSlave::runModbusSlave()
{
    if(!this->isInitSuccess())
        runInfo_.error = SLAVE_Error1;
    this->baseTimerHandler();
    this->slaveErrorHandler();
    switch(this->runInfo_.status)
    {
    case SLAVE_IDLE_STATUS:
        if(this->isCmdPackComeIn())
        {
        	this->prepareForRecv();
            runInfo_.status = SLAVE_RECV_STATUS;
        }
        break;
    case SLAVE_RECV_STATUS:
        if(this->recvCmdPackRTU())
            runInfo_.status = SLAVE_RECV_FINISH;
        break;
    case SLAVE_RECV_FINISH:
        if(this->masterCmdPackHandle())
        {
        	this->prepareForSend();
            runInfo_.status = SLAVE_RSPD_STATUS;
        }
        break;
    case SLAVE_RSPD_STATUS:
        if(this->sendRspPackRTU())
            runInfo_.status = SLAVE_RSPD_FINISH;
        break;
    case SLAVE_RSPD_FINISH:
        runInfo_.status = SLAVE_IDLE_STATUS;
        break;
    case SLAVE_ERROR_STATUS:
        this->clearSlaveError();
        break;
    default:
        runInfo_.status = SLAVE_IDLE_STATUS;
        break;
    }
}
