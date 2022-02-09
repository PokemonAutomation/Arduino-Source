/*  Serial Connection Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QSerialPortInfo>
#include <QHBoxLayout>
#include "Common/Qt/NoWheelComboBox.h"
#include "SerialSelectorWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


SerialSelectorWidget::SerialSelectorWidget(
    QWidget& parent,
    SerialSelector& value,
    Logger& logger
)
    : QWidget(&parent)
    , m_value(value)
    , m_logger(logger, value.m_logger_tag)
    , m_connection(value.m_port, value.m_minimum_pabotbase, m_logger)
{
    QHBoxLayout* serial_row = new QHBoxLayout(this);
    serial_row->setContentsMargins(0, 0, 0, 0);

    serial_row->addWidget(new QLabel("<b>Serial Port:</b>", this), 1);
    serial_row->addSpacing(5);

    m_serial_box = new NoWheelComboBox(this);
    serial_row->addWidget(m_serial_box, 5);
    refresh();
    serial_row->addSpacing(5);

    QWidget* status = new QWidget(this);
    serial_row->addWidget(status, 3);
    QVBoxLayout* sbox = new QVBoxLayout(status);
    sbox->setContentsMargins(0, 0, 0, 0);
    sbox->setSpacing(0);
    m_serial_program = new QLabel(this);
    sbox->addWidget(m_serial_program);
    m_serial_uptime = new QLabel(this);
    sbox->addWidget(m_serial_uptime);
    serial_row->addSpacing(5);

    m_serial_program->setText("<font color=\"red\">Not Connected</font>");
    m_serial_uptime->hide();

    m_reset_button = new QPushButton("Reset Serial", this);
    serial_row->addWidget(m_reset_button, 1);

    connect(
        m_serial_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
        this, [=](int index){
            QSerialPortInfo& current_port = m_value.m_port;
            if (index <= 0 || index > m_ports.size()){
                current_port = QSerialPortInfo();
            }else{
                const QSerialPortInfo& port = m_ports[index - 1];
                if (!current_port.isNull() && current_port.systemLocation() == port.systemLocation()){
                    return;
                }
                current_port = port;
            }
            reset();
        }
    );
    connect(
        &m_connection, &BotBaseHandle::on_not_connected,
        this, [=](QString error){
            if (error.size() <= 0){
                m_serial_program->setText("<font color=\"red\">Not Connected</font>");
            }else{
                m_serial_program->setText(error);
            }
            m_serial_uptime->hide();
        }
    );
    connect(
        &m_connection, &BotBaseHandle::on_connecting,
        this, [=](){
            m_serial_program->setText("<font color=\"green\">Connecting...</font>");
            m_serial_uptime->hide();
        }
    );
    connect(
        &m_connection, &BotBaseHandle::on_ready,
        this, [=](QString description){
            m_serial_program->setText(description);
            on_ready(true);
        }
    );
    connect(
        &m_connection, &BotBaseHandle::on_stopped,
        this, [=](QString error){
            if (error.size() <= 0){
                m_serial_program->setText("<font color=\"orange\">Stopping...</font>");
            }else{
                m_serial_program->setText(error);
            }
        }
    );
    connect(
        &m_connection, &BotBaseHandle::uptime_status,
        this, [=](QString status){
            m_serial_uptime->setText(status);
            m_serial_uptime->show();
        }
    );
    connect(
        m_reset_button, &QPushButton::clicked,
        this, [=](bool){
            reset();
        }
    );
}
SerialSelectorWidget::~SerialSelectorWidget(){}
void SerialSelectorWidget::refresh(){
    m_serial_box->clear();
    m_serial_box->addItem("(none)");

    m_ports.clear();
    for (QSerialPortInfo& port : QSerialPortInfo::availablePorts()){
#ifdef _WIN32
        //  COM1 is never the correct port on Windows.
        if (port.portName() == "COM1"){
            continue;
        }
#endif
        m_ports.emplace_back(std::move(port));
    }

    QSerialPortInfo& current_port = m_value.m_port;

    size_t index = 0;
    for (size_t c = 0; c < m_ports.size(); c++){
        const QSerialPortInfo& port = m_ports[c];
        m_serial_box->addItem(port.portName() + " - " + port.description());

        if (!current_port.isNull() && current_port.systemLocation() == port.systemLocation()){
            index = c + 1;
        }
    }
    if (index != 0){
        m_serial_box->setCurrentIndex((int)index);
    }else{
        current_port = QSerialPortInfo();
        m_serial_box->setCurrentIndex(0);
    }
}

bool SerialSelectorWidget::is_ready() const{
    return m_connection.state() == BotBaseHandle::State::READY;
}
BotBaseHandle& SerialSelectorWidget::botbase(){
    return m_connection;
}
void SerialSelectorWidget::set_options_enabled(bool enabled){
    m_serial_box->setEnabled(enabled);
    m_reset_button->setEnabled(enabled);
}

void SerialSelectorWidget::stop(){
    m_connection.stop();
}
void SerialSelectorWidget::reset(){
    stop();
    on_ready(false);
    refresh();
    m_connection.reset(m_value.m_port);
}





}
