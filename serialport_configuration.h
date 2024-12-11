#pragma once
#include <QString>
#pragma pack(1)
typedef struct
{
    QString dev_name;
    unsigned int baudrate;
    unsigned char data_bit;
    unsigned char check_bit;
    unsigned char stop_bit;
}serialport_configuration;
#pragma pack()