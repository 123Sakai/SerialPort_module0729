#include "SerialPort_module.h"

SerialPort_module::SerialPort_module(QObject *parent)
	: QObject(parent)
{
    serialport = new QSerialPort;
    find_serialport();
    
    //数据接收模式
    timerserial = new QTimer();
    //QObject::connect(serialport, &QSerialPort::readyRead, this, &SerialPort_module::serial_timerstart);
    //QObject::connect(timerserial, &QTimer::timeout, this, &SerialPort_module::get_data);
    connect(serialport, &QSerialPort::readyRead, this, &SerialPort_module::get_data);
    connect(serialport, &QSerialPort::readyRead, this, &SerialPort_module::get_parsed_data);
    //connect()
}


SerialPort_module::~SerialPort_module()
{
    if (serialport->isOpen())
    {
        serialport->close();
    }
    delete serialport;
}

QStringList SerialPort_module::find_serialport()
{
    QStringList comx;
    QList serialinfo = QSerialPortInfo::availablePorts();
    for (QSerialPortInfo& serialPortInfo : serialinfo)
    {
        comx << serialPortInfo.portName();
    }
    return comx;
}

bool SerialPort_module::open_serialport(const serialport_configuration &se_conf)
{
    bool ret = false;
    sleep(100);
    //init serialport
    //this->serial_config = se_conf;
    serialport->setPortName(se_conf.dev_name);
    if (serialport->open(QIODevice::ReadWrite))
    {
        serialport->setBaudRate(se_conf.baudrate);
        switch (se_conf.data_bit)
        {
        case 8:serialport->setDataBits(QSerialPort::Data8); break;
        default: break;
        }
        switch (se_conf.check_bit)
        {
        case 0: serialport->setParity(QSerialPort::NoParity); break;
        case 1: serialport->setParity(QSerialPort::OddParity); break;
        case 2: serialport->setParity(QSerialPort::EvenParity); break;
        default: break;
        }
        switch (se_conf.stop_bit)
        {
        case 1: serialport->setStopBits(QSerialPort::OneStop); break;
        case 2: serialport->setStopBits(QSerialPort::TwoStop); break;
        default: break;
        }
        serialport->setFlowControl(QSerialPort::NoFlowControl);
        //QMessageBox::information(this, tr("Success"), tr("Open successfully."));
        ret = true;
        return ret;
    }
    else
    {
        sleep(100);
        //QMessageBox::information(this, tr("Error"), tr("Open the failure."));
        find_serialport();
        ret = false;
        return ret;
    }
}

bool SerialPort_module::close_serialport()
{
    bool ret = false;
    if (serialport->isOpen())
    {   
        serialport->clear();
        serialport->close();
        ret = true;
    }
    find_serialport();
    return ret;
}

void SerialPort_module::send_ascii_data(QString sen)
{
    if (!sen.isEmpty())
    {
        QByteArray sentmp = sen.toLocal8Bit();
        qDebug() << "send_ascii_data = " << sentmp;
        serialport->write(sentmp);
        sentmp.clear();
    }
    else 
    {
        qDebug() << "send buffer is empty.";
    }
    sen.clear();
    
    sleep(100);
}

void SerialPort_module::send_hex_data(QString sen)
{
    qDebug() << "send_hex_data1 sen = " << sen;
    uint8_t is_r[1024];
    uint8_t is_n[1024];
    uint8_t utmp[2048];
    uint8_t utmp2[1024];// 不写512是因为不好debug
    memset(utmp, 0, sizeof(utmp));
    memset(utmp2, 0, sizeof(utmp2));
    memset(is_r, 0, sizeof(is_r));
    memset(is_n, 0, sizeof(is_n));
    int i = 0, j = 0, k = 0;// i 是utmp的容器长度(不带空格)；j 是sen的容器长度(带空格)；k 是整合以后的数据长度
    int ir = 0, in = 0; // 这两个分别是放is_r和is_n的
    float tlen = 0;// 最终去掉空格后sen的长度 
    int len = 0;// 读到的数据的长度
    len = sen.length();
    if(sen.isEmpty())
    {
        qDebug() << "send buffer is empty.";
    }
    else
    {
        //先toLocal8Bit
        sen = sen.toLocal8Bit();
        qDebug() << "send_hex_data>>utmp toLocal8Bit:" << sen;
        j = 0;
        tlen = 0;
        //后toLatin1
        for (i = 0; i < len; i++)
        {
            if (sen[j] != ' ' && sen[j] != '\r' && sen[j] != '\n')
            {
                utmp[i] = (uint8_t)(sen[j].toLatin1());

                qDebug() << "send_hex_data>>utmp toLatin1:" << utmp[i];
            }
            else if (sen[j] == ' ')
            {
                j++;//sen跳过这个空格
                i--;//缩回
                continue;
            }
            else if (sen[j] == '\r')
            {
                ir++;
                utmp[i] = '\r';
                is_r[ir] = i;
                is_r[0] += 1;
                qDebug() << "send_hex_data>>utmp toLatin1:" << utmp[i];
            }
            else if (sen[j] == '\n')
            {
                in++;
                utmp[i] = '\n';
                is_n[in] = i;
                is_n[0] += 1;
                qDebug() << "send_hex_data>>utmp toLatin1:" << utmp[i];
            }
            j++;
            if (j >= len) // 如果j读到最后一个字节
            {
                tlen += i; // 把sen去掉空格后的数据长度存给tlen
                break;
            }
        }
        //再toHex
        for (i = 0; i < len; i++)
        {
            utmp[i] = ascii_to_hex(utmp[i]);
        }
        //merge
        k = 0;
        for (i = 0; i < (tlen / 2 + 1); i++)
        {
            utmp2[k] = (utmp[i * 2 + 0] << 4) | (utmp[i * 2 + 1]);
            k++;
        }
        if ((is_r[0] != 0))
        {
            for (int m = 1; m <= ir; m++)
            {
                if (is_r[m] % 2 == 1)
                {
                    utmp2[is_r[m] / 2 + 1] = (0x00 << 4) | '\r';
                }
                else
                {
                    utmp2[is_r[m] / 2] = (0x00 << 4) | '\r';
                }
                tlen++;
            }
        }
        if ((is_n[0] != 0))
        {
            for (int m = 1; m <= in; m++)
            {
                if (is_n[m] % 2 == 1)
                {
                    utmp2[is_n[m] / 2 + 1] = (0x00 << 4) | '\n';
                }
                else
                {
                    utmp2[is_n[m] / 2] = (0x00 << 4) | '\n';
                }
                tlen++;
            }
        }

        //sendBuf = sen.toLocal8Bit().toHex().toUpper();
    }
    QByteArray sentmp((char*)utmp2, (tlen / 2 + 1));
    qDebug() << "send_hex_data2 sentmp = " << sentmp;
    serialport->write(sentmp);
    sen.clear();
    sentmp.clear();
    sleep(100);
}

int SerialPort_module::ascii_to_hex(char ch)
{
    int hex = 0;

    if ((ch >= '0') && (ch <= '9'))
    {
        hex = ch - '0';
    }
    else if ((ch >= 'A') && (ch <= 'F'))
    {
        hex = ch - 'A' + 10;
    }
    else if ((ch >= 'a') && (ch <= 'f'))
    {
        hex = ch - 'a' + 10;
    }
    else
    {
        hex = 0;
    }
    return hex;
}

void SerialPort_module::get_data()
{
    timerserial->stop();//停止定时器
    
    QString data = serial_timerstart();
    QByteArray recvbuf = serial_timerstart();
    qDebug() << "get_data>>raw buffer recvbuf 1:" << recvbuf;
    qDebug() << "get_data>>raw buffer recvbuffer 1:" << recvbuffer;
    m_data.clear();
    recvbuffer.clear();
    
    emit dataReceived(recvbuf);
    /*qDebug() << "get_data>>raw buffer data 1:" << data.toLatin1();
    emit dataReceived(data.toLatin1());*/
    //return m_data;
}


QByteArray SerialPort_module::serial_timerstart()
{
    //串口读数据周期时间，t=单个串口最大字节数/波特率(单位:ms)
    timerserial->start(100);
    recvbuffer.append(serialport->readAll());
    return recvbuffer;
}

void SerialPort_module::sleep(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);//结束时间等于当前系统获取到的时间加输入的毫秒数msec
    while (QTime::currentTime() < dieTime)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}


void SerialPort_module::get_parsed_data()
{
    QByteArray task(serial_timerstart());
    if (parseRecvDataInstance != nullptr)
    {
        parseRecvDataInstance->addTask(task.toStdString());
    }
    emit dataReceived_parse(task);
}


