#include "ModbusMaster.h"
#ifdef DEBUG_CODE
//#include "debug.h"
#include "SerialPortHelper.h"
#endif

ModbusMaster::ModbusMaster(MasterInitStruct initStruct):
    heartChkTime_(0),
    currRecvTime_(0),
    reSendCount_(0)
#ifdef DEBUG_CODE
   ,timeCounter_(0)
#endif
{
    this->runInfo_.status = MASTER_IDLE_STATUS;
    this->runInfo_.exceptCode = Except_Code0;
    this->runInfo_.error = Master_Error0;
    this->runInfo_.errTimes = 0;
    this->sendBuff_.hasSend = 0;
    this->sendBuff_.front = 0;
    this->sendBuff_.rear = 0;
    this->recvBuff_.hasRecv = 0;
    this->recvBuff_.dataLen = 0;
    this->recvBuff_.pDArea = NULL;
#ifdef DEBUG_CODE
    this->baseTimer_.pTimeCounter = &timeCounter_;
    this->baseTimer_.countCyclTime = 1;
#else
    this->baseTimer_.pTimeCounter = initStruct.pTimeCounter;
    this->baseTimer_.countCyclTime = initStruct.countCyclTime;
#endif
    this->commTimeOut_ = initStruct.commTimeOut;
    this->reSendTimes_ = initStruct.reSendTimes;
    this->sendBuff_.buffLen = initStruct.sendBuffLen;
    this->recvBuff_.buffLen = initStruct.recvBuffLen;
    this->recvBuff_.recvLen = initStruct.recvBuffLen;
    this->sendBuff_.pBuff = new Uint8[initStruct.sendBuffLen];
    this->recvBuff_.pBuff = new Uint8[initStruct.recvBuffLen];
    this->callBack_.readComDev = initStruct.callBack.readComDev;
    this->callBack_.writeComDev = initStruct.callBack.writeComDev;
    this->callBack_.dataHandler = initStruct.callBack.dataHandler;
    this->callBack_.errorHandler = initStruct.callBack.errorHandler;
    if(sendBuff_.pBuff != NULL
    && recvBuff_.pBuff != NULL
	&& callBack_.readComDev != NULL
	&& callBack_.writeComDev != NULL
	&& callBack_.dataHandler != NULL
	&& callBack_.errorHandler != NULL
    && baseTimer_.pTimeCounter != NULL)
    {
        isInitOK_ = TRUE;
    } else isInitOK_ = FALSE;
#ifdef DEBUG_CODE
    errorToDecrMap_.insert(Master_Error1, "初始化失败");
    errorToDecrMap_.insert(Master_Error2, "发送缓冲区空间不足");
    errorToDecrMap_.insert(Master_Error3, "接收缓冲区空间不足");
    errorToDecrMap_.insert(Master_Error4, "CRC校验错误");
    errorToDecrMap_.insert(Master_Error5, "从机返回异常响应帧");
    errorToDecrMap_.insert(Master_Error6, "通信接收超时");
    errorToDecrMap_.insert(Master_Error7, "获取当前数据包失败");
    QObject::connect(&debugTimer_, SIGNAL(timeout()), this, SLOT(timeOutHandler()));
    debugTimer_.start(1);
#endif
}

ModbusMaster::~ModbusMaster()
{
    delete[] sendBuff_.pBuff;
    delete[] recvBuff_.pBuff;
}

#ifdef DEBUG_CODE
void ModbusMaster::setSerialPort(SerialPortHelper* pSerialPort)
{
    pSerialPort_ = pSerialPort;
//    QObject::connect(pSerialPort_, SIGNAL(recvFinishSignal(Uint8*, Uint16)),
//                     this, SLOT(notifyModbusRecvFinish(Uint8*, Uint16)));
}

void ModbusMaster::timeOutHandler()
{
    this->timeCounter_++;
}
#endif

Uint16 ModbusMaster::createCRC16(Uint8 *str,Uint16 num)
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

void ModbusMaster::baseTimerHandler()
{
    Uint8 incrTime = (*baseTimer_.pTimeCounter)
            *baseTimer_.countCyclTime;
    heartChkTime_ += incrTime;
    currRecvTime_ += incrTime;
    *(baseTimer_.pTimeCounter) = 0;
}

Bool ModbusMaster::isSendBuffEmpty()
{
    return sendBuff_.front == sendBuff_.rear;
}

Bool ModbusMaster::insertElement(SendQueue* pQueue, Uint8 data)
{
    if((pQueue->rear + 1) % pQueue->buffLen == pQueue->front)
        return FALSE; //队列已满时,不执行入队操作
    pQueue->pBuff[pQueue->rear] = data;
    //尾指针指向下一个位置
    pQueue->rear = (pQueue->rear + 1) % pQueue->buffLen;
    return TRUE;
}

Bool ModbusMaster::deleteElement(SendQueue* pQueue, Uint8* pPack)
{
    if(pQueue->front == pQueue->rear)
        return FALSE; //缓冲队列空，直接返回
    *pPack = pQueue->pBuff[pQueue->front];
    pQueue->front = (pQueue->front + 1) % pQueue->buffLen;
    return TRUE;
}

Bool ModbusMaster::insertPack(SendQueue* pQueue, Uint8* pPack, Uint16 packLen)
{
    Uint16 i = 0;
    Uint16 rear = pQueue->rear;
    if(pQueue->pBuff == NULL)
        return FALSE;
    if(!insertElement(pQueue,packLen >> 8)
    || !insertElement(pQueue,packLen & 0x0FF))
    { //插入包长度
        pQueue->rear = rear;
        return FALSE;
    }
    for(i = 0; i < packLen; i++)
    {
        if(!insertElement(pQueue,pPack[i]))
        { //插入包内容
            pQueue->rear = rear;
            return FALSE;
        }
    }
    return TRUE;
}

Bool ModbusMaster::getCurrPack(SendQueue* pQueue, Uint8* pPack, Uint16* pPackLen)
{
    Uint16 i = 0, packLen = 0;
    Uint8 packLenL, packLenH;
    Uint16 front = pQueue->front;
    ;
    if(!deleteElement(pQueue,&packLenH)
    || !deleteElement(pQueue,&packLenL))
    {
        pQueue->front = front;
        return FALSE;
    }
    packLen = packLenL + packLenH*256;
    if(packLen > *pPackLen)
    {
        pQueue->front = front;
        return FALSE;
    }
    for(i = 0; i < packLen; i++)
    {
        if(!deleteElement(pQueue,pPack + i))
        {
            pQueue->front = front;
            return FALSE;
        }
    }
    *pPackLen = packLen;
    pQueue->front = front;
    return TRUE;
}

Bool ModbusMaster::deleteCurrPack(SendQueue* pQueue)
{
    Uint16 i = 0, packLen = 0;
    Uint8 packLenL, packLenH;
    Uint16 front = pQueue->front;
    if(!deleteElement(pQueue,&packLenH)
    || !deleteElement(pQueue,&packLenL))
    {
        pQueue->front = front;
        return FALSE;
    }
    packLen = packLenL + packLenH*256;
    for(i = 0; i < packLen; i++)
    {
        if(!deleteElement(pQueue,&packLenH))
        {
            pQueue->front = front;
            return FALSE;
        }
    }
    return TRUE;
}


Bool ModbusMaster::toFunCode03CmdPackRTU(Uint8 slaveID, Uint16 startReg,  Uint16 regNum)
{
    Bool retVal = FALSE;
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
    retVal = insertPack(&sendBuff_, packBuff, packLen);
    if(retVal == FALSE) this->runInfo_.error = Master_Error2;
    return retVal;
}

Bool ModbusMaster::toFunCode06CmdPackRTU(Uint8 slaveID, Uint16 startReg, Uint16 data)
{
    Bool retVal = FALSE;
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
    retVal = insertPack(&sendBuff_, packBuff, packLen);
    if(retVal == FALSE) this->runInfo_.error = Master_Error2;
    return retVal;
}

void ModbusMaster::exceptRspHander(Uint8* pBuff)
{
    ExceptionRSP *pFrame;
    pFrame = (ExceptionRSP*)pBuff;
    this->runInfo_.exceptCode = pFrame->except;
    this->runInfo_.error = Master_Error5;
}

Bool ModbusMaster::readDataFromSlave(Uint8 slaveID, Uint16 startAddr, Uint16 dataNum)
{
    if(this->runInfo_.status == MASTER_ERROR_STATUS
    || this->runInfo_.error != Master_Error0)
        return FALSE;
    return toFunCode03CmdPackRTU(slaveID, startAddr, dataNum);
}

Bool ModbusMaster::writeDataToSlave(Uint8 slaveID, Uint16 addr, Uint16 data)
{
    if(this->runInfo_.status == MASTER_ERROR_STATUS
    || this->runInfo_.error != Master_Error0)
        return FALSE;
    return toFunCode06CmdPackRTU(slaveID, addr, data);
}

void ModbusMaster::prepareForSend()
{
    sendBuff_.hasSend = 0;
}

Bool ModbusMaster::sendCurrCmdPackRTU()
{
    Bool retVal = FALSE;
    Sint16 wrRetVal = 0;
    Uint16 packLen = 50;
    static Uint8 currPack[50]; //当前数据包发送缓冲区
    Uint16 hasSend = sendBuff_.hasSend;
    retVal = getCurrPack(&sendBuff_, currPack, &packLen);
    if(retVal != TRUE)
    {
        runInfo_.error = Master_Error7;
        return FALSE;
    }
    retVal = FALSE;
    if(sendBuff_.hasSend < packLen)
    {
        wrRetVal = callBack_.writeComDev
        		(currPack + hasSend, packLen - hasSend);
        sendBuff_.hasSend +=
        		((wrRetVal > 0) ? wrRetVal : 0);
    }
    else
    { //发送完成
        retVal = TRUE;
#ifdef DEBUG_CODE
        pSerialPort_->showInCommBrowser(QString("Error Times: "),
                      QString::number(this->runInfo_.errTimes));
        pSerialPort_->showInCommBrowser(QString("发送内容:"),
                      QByteArray((char *)currPack, packLen));
//        debug("Error Times: %d",this->runInfo_.errTimes);
//        debug("发送内容:");
//        int i = 0;
//        for(i = 0; i < dataLen; i++)
//        {
//            debug("%d ",pPackData[i]);
//        }
//        debug("\n\n\n");
#endif
    }
    return retVal;
}

Bool ModbusMaster::getCmdPackHeader(Uint8* pHeader)
{
    Uint16 front = this->sendBuff_.front;
    if(front == this->sendBuff_.rear)
        return FALSE;
    pHeader[0] = this->sendBuff_.pBuff[front + 2];
    pHeader[1] = this->sendBuff_.pBuff[front + 3];
    return TRUE;
}

Bool ModbusMaster::getRspPackHeader()
{
    Bool retVal = FALSE;
    Uint8 cmdPackHeader[2];
    Uint16 i, foundPos = 0;
    Uint16 hasRecv = this->recvBuff_.hasRecv;
    Uint8* pBuff = this->recvBuff_.pBuff;
    if(!getCmdPackHeader(cmdPackHeader))
        return FALSE;
    for(i = 0; i < hasRecv - 1; i++)
    {
        if(cmdPackHeader[0] == pBuff[i] &&
           cmdPackHeader[1] == (pBuff[i+1] & 0x7F))
        {
            retVal = TRUE;break;
        }
    }
    foundPos = i;
    if(foundPos != 0)
    { //去掉无效数据，将包头对齐到RecvBuff首部
        this->recvBuff_.hasRecv = 0;
        for(i = foundPos; i < hasRecv; i++)
        {
            this->recvBuff_.hasRecv++;
            pBuff[i - foundPos] = pBuff[i];
        }
    }
    return retVal;
}

Bool ModbusMaster::getRspPackLen(Uint8* pPackLen)
{
    Bool retVal = FALSE;
    Uint8 dataLen = 0;
    Uint8* pBuff = this->recvBuff_.pBuff;
    switch(pBuff[1])
    {
    case READ_MUL_HLD_REG:
        if(this->recvBuff_.hasRecv >= 3)
        {
            retVal = TRUE;dataLen = pBuff[2];
            *pPackLen = 3 + dataLen + 2;
        }
        break;
    case WRITE_ONE_HLD_REG:
        retVal = TRUE;
        *pPackLen = sizeof(WriteOneRegRSP_X06)
                /sizeof(Uint8);
        break;
    default:
        if(pBuff[1] >= 0x80)
        { //从机返回异常功能码
            retVal = TRUE;
            *pPackLen = sizeof(ExceptionRSP)
                    /sizeof(Uint8);
        }
        break;
    }
    return retVal;
}

void ModbusMaster::prepareForRecv()
{
	currRecvTime_  = 0;
	recvBuff_.hasRecv = 0;
}

Bool ModbusMaster::recvRspPackRTU()
{
    Bool retVal = FALSE;
    Sint16 rdRetVal = 0;
    static Uint8 recvStep = 1;
    Uint8* buff = recvBuff_.pBuff;
    Uint16 recvLen = recvBuff_.recvLen;
    Uint16 hasRecv = recvBuff_.hasRecv;
    static Uint8 packLen = recvBuff_.recvLen;
    if(recvLen > recvBuff_.buffLen
    || hasRecv >= recvLen)
    {
        runInfo_.error = Master_Error3;
        return FALSE;
    }
    if(this->currRecvTime_ > this->commTimeOut_)
    {
    	this->runInfo_.error = Master_Error6;
    	recvStep = 1;return FALSE;
    }
    if(hasRecv < packLen)
    {
        rdRetVal = callBack_.readComDev
                (buff + hasRecv, recvLen - hasRecv);
        recvBuff_.hasRecv +=
        		((rdRetVal > 0) ? rdRetVal : 0);
    }
    switch(recvStep)
    {
    case 1:
        if(getRspPackHeader())
            recvStep = 2;
        else break;
    case 2:
        if(getRspPackLen(&packLen))
            recvStep = 3;
        else break;
    case 3:
        if(hasRecv >= packLen)
        {
            recvStep = 1;
            retVal = TRUE;
#ifdef DEBUG_CODE
			pSerialPort_->showInCommBrowser(QString("接收内容:"),
						  QByteArray((char *)this->recvBuff_.pBuff,
						  this->recvBuff_.hasRecv));
//			debug("接收内容:");
//			int i = 0;
//			for(i = 0; i < this->recvBuff_.recvLen; i++)
//			{
//				debug("%d ",this->recvBuff_.pBuff[i]);
//			}
//			debug("\n\n\n");
#endif
        }
        else break;
    default: recvStep = 1;
    break;
    }
    return retVal;
}

Sint16 ModbusMaster::getDataDescr()
{
    Uint8 pack[50];
    Bool retVal = FALSE;
    Uint16 dataDescr, packLen;
    retVal = getCurrPack(&sendBuff_, pack, &packLen);
    if(retVal == FALSE) return -1;
#if   (TRF_ORDER == 0)
    dataDescr = pack[2] << 8;
    dataDescr |= pack[3] & 0x0FF;
#elif (TRF_ORDER == 1)
    dataDescr = pack[3] << 8;
    dataDescr |= pack[2] & 0x0FF;
#endif
    return dataDescr;
}

Bool ModbusMaster::handleRspPackRTU()
{
    Uint16 crc16, crc16Recv;
    Uint16 i = 0, dataLen = 0;
    Uint16 dataTmp, *pDataTmp = NULL;
    Uint16 packLen = this->recvBuff_.hasRecv;
    Uint8* pBuff = this->recvBuff_.pBuff;
    this->recvBuff_.dataLen = 0;
    crc16 = createCRC16(pBuff, packLen - 2);
    crc16Recv = (pBuff[packLen - 1] << 8)
              | (pBuff[packLen - 2] & 0x0FF);
    if(crc16 != crc16Recv)
    {
        this->runInfo_.error = Master_Error4;
        return FALSE;
    }
    if(pBuff[1] >= 0x80)
    { //从机返回异常功能码
        exceptRspHander(pBuff);
        return FALSE;
    }
    switch(pBuff[1])
    {
    case READ_MUL_HLD_REG:
        pDataTmp = (Uint16*)(pBuff + 3);
        dataLen = pBuff[2];
        for(i = 0; i < dataLen/2; i++)
        {
#if   (TRF_ORDER == 0)
            dataTmp = (pBuff[3+i*2] << 8);
            dataTmp |= pBuff[3+i*2+1];
#elif (TRF_ORDER == 1)
            dataTmp = (pBuff[3+i*2+1] << 8);
            dataTmp |= pBuff[3+i*2];
#endif
            pDataTmp[i] = dataTmp;
        }
        this->recvBuff_.dataLen = dataLen/2;
        this->recvBuff_.pDArea = pDataTmp;
        this->recvBuff_.dataDecr = getDataDescr();
        break;
    case WRITE_ONE_HLD_REG:
        break;
    default:break;
    }
    if(this->recvBuff_.dataLen > 0)
    {
#ifdef DEBUG_CODE
        pSerialPort_->showInCommBrowser(QString("解析结果:"),
                      QByteArray((char *)this->recvBuff_.pDArea, this->recvBuff_.dataLen*2));
//        debug("解析结果:");
//        int i = 0;
//        for(i = 0; i < this->recvBuff_.dataLen; i++)
//        {
//            debug("%d ",this->recvBuff_.pDataArea[i]);
//        }
//        debug("\n\n\n");
#endif
        this->callBack_.dataHandler(this->recvBuff_.pDArea,
                                    this->recvBuff_.dataLen,
                                    this->recvBuff_.dataDecr);
    }
    return TRUE;
}

void ModbusMaster::clearMasterError()
{
    if(this->runInfo_.status == MASTER_RECV_FINISH
    || this->runInfo_.status == MASTER_ERROR_STATUS)
    {
        this->runInfo_.status = MASTER_IDLE_STATUS;
        this->reSendCount_ = 0;
        this->runInfo_.error = 0;
        this->runInfo_.exceptCode = 0;
        deleteCurrPack(&sendBuff_);
    }
}

void ModbusMaster::heartbeatDetect()
{
    if(heartChkTime_ > 3000)
    {
        heartChkTime_ = 0;
        this->readDataFromSlave(1, 0x01, 1);//心跳包
//        this->readDataFromSlave(1, 0x0300C, 1);//心跳包
    }
}

void ModbusMaster:: appErrorHandler()
{
    this->callBack_.errorHandler();
}

void ModbusMaster::masterErrorHandler()
{
    if(this->runInfo_.status == MASTER_ERROR_STATUS
    || this->runInfo_.error == Master_Error0)
        return;
    this->runInfo_.errTimes++;
    if(this->runInfo_.error == Master_Error4)
    {
        if(this->reSendCount_++ < this->reSendTimes_)
        {
            this->runInfo_.error = Master_Error0;
            this->runInfo_.status = MASTER_SEND_STATUS;
        }
        else
            this->runInfo_.status = MASTER_ERROR_STATUS;
    }
    else
        this->runInfo_.status = MASTER_ERROR_STATUS;
#ifdef DEBUG_CODE
    if(this->runInfo_.error != Master_Error0)
    {
        pSerialPort_->showInCommBrowser(QString("Error: ")
                     .append(QString::number(this->runInfo_.error)),
                      QString(errorToDecrMap_.value(this->runInfo_.error)));
//        debug("Error: %d", this->runInfo_.error);
//        debug("\n\n\n");
    }
#endif
}

void ModbusMaster::runModbusMaster()
{
    if(!this->isInitSuccess())
        runInfo_.error = Master_Error1;
    this->baseTimerHandler();
    this->masterErrorHandler();
    switch (this->runInfo_.status)
    {
    case MASTER_IDLE_STATUS:
        if(!this->isSendBuffEmpty())
        {
        	this->prepareForSend();
            this->runInfo_.status = MASTER_SEND_STATUS;
        }
        else
            this->heartbeatDetect();
        break;
    case MASTER_SEND_STATUS:
        if(this->sendCurrCmdPackRTU())
        {
        	this->prepareForRecv();
            this->runInfo_.status = MASTER_RECV_STATUS;
        }
        break;
    case MASTER_RECV_STATUS:
        if(this->recvRspPackRTU())
            this->runInfo_.status = MASTER_RECV_FINISH;
        break;
    case MASTER_RECV_FINISH:
        if(this->handleRspPackRTU())
            this->clearMasterError();
        break;
    case MASTER_ERROR_STATUS:
        if(this->runInfo_.error != Master_Error0)
            this->appErrorHandler();
        //清除错误，防止pCommErrorHandler反复调用
        this->runInfo_.error = Master_Error0;
        break;
    default:
        this->runInfo_.status = MASTER_IDLE_STATUS;
        break;
    }
}
