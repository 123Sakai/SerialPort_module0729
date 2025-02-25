#include "SerialPort_module0729.h"


SerialPort_module0729::SerialPort_module0729(QWidget *parent)
    : QMainWindow(parent),m_serialPort(new SerialPort_module(this))
{
    ui.setupUi(this);
    QCoreApplication::setApplicationName("Serialsvlz");
    QCoreApplication::setApplicationVersion("V8.13");
    this->setWindowIcon(QIcon(":/img/3c7.ico"));
    setWindowTitle("串口V8.13");
    on_findButton_clicked();
    
    connect(ui.refreshbtn, SIGNAL(clicked()), this, SLOT(on_findButton_clicked()));
    connect(ui.open_serial, SIGNAL(clicked()), this, SLOT(on_openButton_clicked()));
    connect(ui.off_serial, SIGNAL(clicked()), this, SLOT(on_closeButton_clicked()));
    connect(ui.push_button, SIGNAL(clicked()), this, SLOT(on_sendButton_clicked()));
    connect(ui.clear_button, SIGNAL(clicked()), this, SLOT(on_clearButton_clicked()));
    //connect(m_serialPort, &SerialPort_module::com_list, this, &SerialPort_module0729::update_comxlist);
    connect(m_serialPort, &SerialPort_module::dataReceived, this, &SerialPort_module0729::recv_data);
    //connect(m_serialPort, &SerialPort_module::dataReceived_parse, this, &SerialPort_module0729::recv_data);
    
    ui.open_serial->setEnabled(true);
    ui.off_serial->setEnabled(false);
    ui.clear_button->setEnabled(true);
    ui.push_button->setEnabled(false);
}

SerialPort_module0729::~SerialPort_module0729()
{
    m_serialPort->~SerialPort_module();
}

void SerialPort_module0729::on_findButton_clicked()
{
    QStringList comx;
    comx = m_serialPort->find_serialport();
    update_comxlist(comx);
    //comx = m_serialPort->find_serialport();
    //for (int i = 0; i < comx.count(); i++)
    //{
    //    ui.com->addItem(comx[i]);
    //}
}
void SerialPort_module0729::update_comxlist(const QStringList& comx)
{
    ui.com->clear();
    ui.com->addItems(comx);
}
void SerialPort_module0729::on_openButton_clicked()
{
    bool ret = false;
    serial_conf.dev_name = ui.com->currentText();
    serial_conf.baudrate = ui.baudrate->currentText().toInt();
    serial_conf.data_bit = static_cast<QSerialPort::DataBits>(ui.databit->currentIndex());
    serial_conf.check_bit = static_cast<QSerialPort::StopBits>(ui.checkbit->currentIndex());
    serial_conf.stop_bit = static_cast<QSerialPort::Parity>(ui.stopbit->currentIndex());
    ret = m_serialPort->open_serialport(serial_conf);
    if (ret)
    {
        ui.open_serial->setEnabled(false);
        ui.off_serial->setEnabled(true);
        ui.clear_button->setEnabled(true);
        ui.push_button->setEnabled(true);
    }
}
void SerialPort_module0729::on_closeButton_clicked()
{
    bool ret = false;
    ret = m_serialPort->close_serialport();
    if (ret)
    {
        ui.open_serial->setEnabled(true);
        ui.off_serial->setEnabled(false);
        ui.clear_button->setEnabled(true);
        ui.push_button->setEnabled(false);
    }
}
void SerialPort_module0729::on_sendButton_clicked()
{
    QString send_data;
    send_data = ui.send_text_window->toPlainText();
    qDebug() << " ";
    qDebug() << "SerialPort_module0729::on_sendButton_clicked raw buffer:" << send_data;
    if (ui.ishex_send->isChecked() == false) // 文本发送
    {
        m_serialPort->send_ascii_data(send_data);
    }
    else if (ui.ishex_send->isChecked() == true) // hex发送
    {
        ui.isuse_thread->clicked(false);
        m_serialPort->send_hex_data(send_data);
    }
    send_data.clear();
}
void SerialPort_module0729::on_clearButton_clicked()
{
    ui.receive_text_window->clear();
}

void SerialPort_module0729::recv_data(const QString &data)
{
    qDebug() << "SerialPort_module0729::recv QString data = " << data;
    
    QDateTime currentTime = QDateTime::currentDateTime();
    QString currentTimeString = currentTime.toString(Qt::ISODate);
    if (ui.ishex_recv->isChecked() && (ui.isuse_thread->isChecked() == false)) //不带解析的hex输出
    {
        qDebug() << "SerialPort_module0729::recv data = " << data;
        ui.isuse_thread->clicked(false); //判断有没有勾选线程解析，如果有，就取消这个勾选
        ui.receive_text_window->append(currentTimeString + "\r\n" + data);
    }
    else if((ui.ishex_recv->isChecked() == false) && (ui.isuse_thread->isChecked() == false)) // 不带解析的普通输出
    { 
        QByteArray test_byte = data.toUtf8();
        qDebug() << "SerialPort_module0729::recv QByteArray test_byte = " << test_byte;
        ui.receive_text_window->append(currentTimeString + "\r\n" + test_byte);
    }
    else if((ui.ishex_recv->isChecked() == false) && ui.isuse_thread->isChecked()) // 带解析的输出
    {
        std::string str1 = data.toStdString();
        std::vector<int> results(10, -1);

        parser.startWorkers(str1, results);
        ui.receive_text_window->append(currentTimeString + "\r\n" + data.toLocal8Bit());
    }
    
}