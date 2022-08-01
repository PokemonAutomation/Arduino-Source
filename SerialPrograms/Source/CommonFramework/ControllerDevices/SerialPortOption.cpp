/*  Serial Connection Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QSerialPortInfo>
#include "Common/Cpp/Pimpl.tpp"
#include "Common/Cpp/Json/JsonValue.h"
#include "SerialPortOption.h"
#include "SerialPortWidget.h"

namespace PokemonAutomation{


SerialPortOption::~SerialPortOption(){}
SerialPortOption::SerialPortOption(PABotBaseLevel minimum_pabotbase)
    : m_minimum_pabotbase(minimum_pabotbase)
    , m_port(CONSTRUCT_TOKEN)
{}
void SerialPortOption::load_json(const JsonValue& json){
    const std::string* name = json.get_string();
    if (name == nullptr || name->empty()){
        return;
    }
    m_port.reset(QSerialPortInfo(QString::fromStdString(*name)));
}
JsonValue SerialPortOption::to_json() const{
    return m_port && !m_port->isNull() ? m_port->portName().toStdString() : "";
}

const QSerialPortInfo* SerialPortOption::port() const{
    if (!m_port || m_port->isNull()){
        return nullptr;
    }
    return m_port.get();
}







}




