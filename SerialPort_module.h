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
#include <queue>
#include <condition_variable>
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

class parseRecvData
{
public:
    parseRecvData() : stop(false) {}

    void startWorkers(const std::string& data, std::vector<int>& results)
    {
        for (int i = 0; i < 4; i++)
        {
            threads.emplace_back(&parseRecvData::worker, this, i, std::ref(results), std::ref(data));
        }

        //// 等待所有线程完成
        //for (auto& t : threads)
        //{
        //    if (t.joinable())
        //    {
        //        t.join();
        //    }
        //}
    }

    // 添加任务到队列中，并通知一个等待的线程
    void addTask(const std::string& task) {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            tasks.push(task);
        }
        cv.notify_one();
    }
    // 请求所有工作线程停止
    void requestStop() {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            stop = true;
        }
        cv.notify_all(); // 唤醒所有可能在等待的任务处理线程
    }

    ~parseRecvData() {
        // 设置停止标志并通知所有等待的线程
        requestStop();
        // 等待所有线程完成
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

private:
    std::mutex queueMutex;
    std::condition_variable cv;
    std::queue<std::string> tasks;
    bool stop = false;
    std::vector<std::thread> threads;

    void worker(int id, std::vector<int>& results, const std::string& data)
    {
        while (true) {
            std::string task;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                cv.wait(lock, [this] { return stop || !tasks.empty(); });
                if (stop && tasks.empty()) {
                    return;
                }
                task = std::move(tasks.front());
                tasks.pop();
            }
            parseDataAndWriteResult(id, results, task); // 开始解析，可调用自己写的解析函数
        }
    }

    void parseDataAndWriteResult(int id, std::vector<int>& results, const std::string& data)
    {
        int result = static_cast<int>(data.length());

        std::lock_guard<std::mutex> lock(queueMutex);
        if (id >= 0 && id < results.size())
        { // 确保 id 在范围内
            results[id] = result;
        }
        qDebug() << "result = " << result;
    }

};

class SerialPort_module  : public QObject
{
	Q_OBJECT

public:
    //SerialPort_module(const SerialPort_module&) = delete;
    //SerialPort_module& operator=(const SerialPort_module&) = delete;
    //SerialPort_module(SerialPort_module&&) = delete;
    //SerialPort_module& operator=(SerialPort_module&&) = delete;

	SerialPort_module(QObject *parent = nullptr);
	~SerialPort_module();
	QTimer* timerserial;
	//QByteArrayData* recvData;
	//QByteArrayData* sendData;
    //std::vector<int>& m_results;

    int ascii_to_hex(char ch);
    void sleep(int msec);
    void setParseRecvData(parseRecvData* prd) {
        parseRecvDataInstance = prd;
    }
    
    
    QStringList find_serialport();
    bool open_serialport(const serialport_configuration& se_conf);
    bool close_serialport();
    void send_hex_data(QString sen);
    void send_ascii_data(QString sen);
    void get_data();
    void get_parsed_data();
    QByteArray serial_timerstart();

signals:
//    void com_list(const QStringList& comx);
    void dataReceived(const QByteArray& data);
    void dataReceived_parse(const QByteArray& data);

private:
    SerialPort_module() :m_dev_name(""), m_baudrate(115200),
        m_data_bit(8), m_check_bit(0), m_stop_bit(1) {}
    QSerialPort* serialport;
    std::string m_dev_name;
    unsigned int m_baudrate;
    unsigned char m_data_bit;
    unsigned char m_check_bit;
    unsigned char m_stop_bit;
    QString m_data;
    QByteArray recvbuffer;
    int count = 0;
    serialport_configuration serial_config;
    parseRecvData* parseRecvDataInstance = nullptr;
};
