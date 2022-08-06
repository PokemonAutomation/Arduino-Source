/*  Serial Connection Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QSerialPortInfo>
#include <QHBoxLayout>
#include "Common/Qt/NoWheelComboBox.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "SerialPortWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


SerialPortWidget::SerialPortWidget(
    QWidget& parent,
    SerialPortSession& session,
    Logger& logger
)
    : QWidget(&parent)
    , m_session(session)
//    , m_logger(logger, GlobalSettings::instance().LOG_EVERYTHING)
//    , m_connection(m_logger, *value.m_port, value.m_minimum_pabotbase)
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
            SerialPortOption& option = m_session.option();
            const QSerialPortInfo* current_port = option.port();
            if (index <= 0 || (size_t)index > m_ports.size()){
                option.clear();
            }else{
                const QSerialPortInfo& port = m_ports[index - 1];
                if (current_port && current_port->systemLocation() == port.systemLocation()){
                    return;
                }
                option.set_port(port);
            }
            reset();
        }
    );
    connect(
        &m_session.botbase(), &BotBaseHandle::on_not_connected,
        this, [=](std::string error){
            if (error.size() <= 0){
                m_serial_program->setText("<font color=\"red\">Not Connected</font>");
            }else{
                m_serial_program->setText(QString::fromStdString(error));
            }
            m_serial_uptime->hide();
        }
    );
    connect(
        &m_session.botbase(), &BotBaseHandle::on_connecting,
        this, [=](){
            m_serial_program->setText("<font color=\"green\">Connecting...</font>");
            m_serial_uptime->hide();
        }
    );
    connect(
        &m_session.botbase(), &BotBaseHandle::on_ready,
        this, [=](std::string description){
            m_serial_program->setText(QString::fromStdString(description));
            emit signal_on_ready(true);
        }
    );
    connect(
        &m_session.botbase(), &BotBaseHandle::on_stopped,
        this, [=](std::string error){
            if (error.size() <= 0){
                m_serial_program->setText("<font color=\"orange\">Stopping...</font>");
            }else{
                m_serial_program->setText(QString::fromStdString(error));
            }
        }
    );
    connect(
        &m_session.botbase(), &BotBaseHandle::uptime_status,
        this, [=](std::string status){
            m_serial_uptime->setText(QString::fromStdString(status));
            m_serial_uptime->show();
        }
    );
    connect(
        m_reset_button, &QPushButton::clicked,
        this, [=](bool){
            reset();
        }
    );

    m_serial_program->setText(QString::fromStdString(m_session.botbase().label()));
    m_session.add_listener(*this);
}
SerialPortWidget::~SerialPortWidget(){
    m_session.remove_listener(*this);
}
void SerialPortWidget::refresh(){
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

    SerialPortOption& option = m_session.option();
    const QSerialPortInfo* current_port = option.port();

    size_t index = 0;
    for (size_t c = 0; c < m_ports.size(); c++){
        const QSerialPortInfo& port = m_ports[c];
        m_serial_box->addItem(port.portName() + " - " + port.description());

        if (current_port && current_port->systemLocation() == port.systemLocation()){
            index = c + 1;
        }
    }
    if (index != 0){
        m_serial_box->setCurrentIndex((int)index);
    }else{
        option.clear();
        m_serial_box->setCurrentIndex(0);
    }
}
void SerialPortWidget::on_ready(bool ready){
    emit signal_on_ready(ready);
}

bool SerialPortWidget::is_ready() const{
    return m_session.is_ready();
}
BotBaseHandle& SerialPortWidget::botbase(){
    return m_session.botbase();
}
void SerialPortWidget::set_options_enabled(bool enabled){
    m_serial_box->setEnabled(enabled);
    m_reset_button->setEnabled(enabled);
}

void SerialPortWidget::stop(){
    m_session.stop();
}
void SerialPortWidget::reset(){
    m_session.reset();
    refresh();
}





}
