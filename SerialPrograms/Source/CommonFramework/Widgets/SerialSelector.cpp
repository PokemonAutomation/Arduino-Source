/*  Serial Connection Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonObject>
#include <QHBoxLayout>
#include "Common/Qt/QtJsonTools.h"
#include "Common/Qt/NoWheelComboBox.h"
#include "SerialSelector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


SerialSelector::SerialSelector(
    QString label, std::string logger_tag,
    PABotBaseLevel minimum_pabotbase
)
    : m_label(std::move(label))
    , m_minimum_pabotbase(minimum_pabotbase)
    , m_logger_tag(std::move(logger_tag))
{}
SerialSelector::SerialSelector(
    QString label, std::string logger_tag,
    PABotBaseLevel minimum_pabotbase,
    const QJsonValue& json
)
    : SerialSelector(std::move(label), std::move(logger_tag), minimum_pabotbase)
{
    load_json(json);
}
void SerialSelector::load_json(const QJsonValue& json){
    QString name = json.toString();
    if (name.size() > 0){
        m_port = QSerialPortInfo(name);
    }
}
QJsonValue SerialSelector::to_json() const{
    return QJsonValue(m_port.isNull() ? "" : m_port.portName());
}

const QSerialPortInfo* SerialSelector::port() const{
    if (m_port.isNull()){
        return nullptr;
    }
    return &m_port;
}

SerialSelectorUI* SerialSelector::make_ui(QWidget& parent, Logger& logger){
    return new SerialSelectorUI(parent, *this, logger);
}




SerialSelectorUI::SerialSelectorUI(
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
SerialSelectorUI::~SerialSelectorUI(){}
void SerialSelectorUI::refresh(){
    m_serial_box->clear();
    m_serial_box->addItem("(none)");

    m_ports = QSerialPortInfo::availablePorts();
    QSerialPortInfo& current_port = m_value.m_port;

    int index = 0;
    for (int c = 0; c < m_ports.size(); c++){
        const QSerialPortInfo& port = m_ports[c];
        m_serial_box->addItem(port.portName() + " - " + port.description());

        if (!current_port.isNull() && current_port.systemLocation() == port.systemLocation()){
            index = c + 1;
        }
    }
    if (index != 0){
        m_serial_box->setCurrentIndex(index);
    }else{
        current_port = QSerialPortInfo();
        m_serial_box->setCurrentIndex(0);
    }
}

bool SerialSelectorUI::is_ready() const{
    return m_connection.state() == BotBaseHandle::State::READY;
}
BotBaseHandle& SerialSelectorUI::botbase(){
    return m_connection;
}
void SerialSelectorUI::set_options_enabled(bool enabled){
    m_serial_box->setEnabled(enabled);
    m_reset_button->setEnabled(enabled);
}

void SerialSelectorUI::stop(){
    m_connection.stop();
}
void SerialSelectorUI::reset(){
    stop();
    on_ready(false);
    refresh();
    m_connection.reset(m_value.m_port);
}



}




