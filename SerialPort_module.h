#pragma once

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QRegularExpression>
#include <QCoreApplication>
#include <QEventLoop>
#include <QString>
#include <qDebug>
#include <QTime>
#include <QTimer>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <ctime>
#include <algorithm>
#include <cinttypes>
#include <thread>
#include <mutex>
#include "serialport_configuration.h"
//#pragma pack(1)
//typedef struct
//{
//    const char* dev_name;
//    const unsigned int baudrate;
//    const unsigned char data_bit;
//    const unsigned char check_bit;
//    const unsigned char stop_bit;
//}m_serialport_t;
//#pragma pack()
class SerialPort_module  : public QObject
{
	Q_OBJECT

public:
    SerialPort_module(const SerialPort_module&) = delete;
    SerialPort_module& operator=(const SerialPort_module&) = delete;
    SerialPort_module(SerialPort_module&&) = delete;
    SerialPort_module& operator=(SerialPort_module&&) = delete;

	SerialPort_module(QObject *parent = nullptr);
	~SerialPort_module();
	QTimer* timerserial;
	//QByteArrayData* recvData;
	//QByteArrayData* sendData;

    int ascii_to_hex(char ch);
    void sleep(int msec);
    
    
    
    QStringList find_serialport();
    bool open_serialport(const serialport_configuration& se_conf);
    bool close_serialport();
    void send_hex_data(QString sen);
    void send_ascii_data(QString sen);
    void get_data();
    QString serial_timerstart();

signals:
//    void com_list(const QStringList& comx);
    void dataReceived(const QString& data);

private:
    SerialPort_module() :m_dev_name(""), m_baudrate(115200),
        m_data_bit(1), m_check_bit(0), m_stop_bit(1) {}
    QSerialPort* serialport;
    std::string m_dev_name;
    unsigned int m_baudrate;
    unsigned char m_data_bit;
    unsigned char m_check_bit;
    unsigned char m_stop_bit;
    QString m_data;
    int count = 0;
    serialport_configuration serial_config;
};
class data_transmit;

class data_transmit : public QObject
{
    Q_OBJECT

public:
    QByteArrayData* recvData;
    QByteArrayData* sendData;
    //std::mutex thrmtx;//static std::mutex thrmtx;
    //bool thrd_start();
    //bool thrd_stop();
    //bool thrd_running();

};