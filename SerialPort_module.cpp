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
    qDebug() << "send_hex_data1 sen= " << sen;
    uint8_t utmp[1024];
    uint8_t utmp2[50];
    memset(utmp, 0, sizeof(utmp));
    memset(utmp2, 0, sizeof(utmp2));
    int i, j, k;
    float tlen = 0;
    int len = 0;
    len = sen.length();
    if(sen.isEmpty())
    {
        qDebug() << "send buffer is empty.";
    }
    else
    {
        sen = sen.toLocal8Bit();
        j = 0;
        tlen = 0;
        for (i = 0; i < len; i++)
        {
            if (sen[j] != ' ')
            {
                utmp[i] = (uint8_t)(sen[j].toLatin1());
            }
            else if (sen[j] == ' ')
            {
                j++;
                i--;
                continue;
            }
            j++;
            if (j >= len)
            {
                tlen = i;
                break;
            }
        }
        for (i = 0; i < len; i++)
        {
            utmp[i] = ascii_to_hex(utmp[i]);
        }
        k = 0;
        for (i = 0; i < (tlen / 2 + 1); i++)
        {
            utmp2[k] = (utmp[i * 2 + 0] << 4) | (utmp[i * 2 + 1]);
            k++;
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
    m_data.clear();
    qDebug() << "get_data>>raw buffer1:" << data;
    emit dataReceived(data);
    //return m_data;
}


QString SerialPort_module::serial_timerstart()
{
    //串口读数据周期时间，t=单个串口最大字节数/波特率(单位:ms)
    timerserial->start(100);
    m_data.append(serialport->readAll());
    return m_data;
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
    QString task(serial_timerstart());
    if (parseRecvDataInstance != nullptr)
    {
        parseRecvDataInstance->addTask(task.toStdString());
    }
    emit dataReceived_parse(task);
}


