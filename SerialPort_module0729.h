#pragma once

#include <QtWidgets/QMainWindow>
#include <QDebug>
#include <QString>

#include<thread>
#include <string>
#include <vector>

#include "ui_SerialPort_module0729.h"
#include "SerialPort_module.h"
#include "serialport_configuration.h"


class SerialPort_module;

class SerialPort_module0729 : public QMainWindow
{
    Q_OBJECT

public:
    SerialPort_module0729(QWidget *parent = nullptr);
    ~SerialPort_module0729();
    serialport_configuration serial_conf = { 0 };
private slots:
    void on_findButton_clicked();
    void on_openButton_clicked();
    void on_closeButton_clicked();
    void on_sendButton_clicked();
    void on_clearButton_clicked();
    void update_comxlist(const QStringList& comx);
    void recv_data(const QString &data);

    
private:
    Ui::SerialPort_module0729Class ui;
    SerialPort_module* m_serialPort;
    parseRecvData parser;
    //std::vector<int>& m_results;
};