/*  Serial Connection Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QSerialPortInfo>
#include "Common/Cpp/Pimpl.tpp"
#include "Common/Cpp/Json/JsonValue.h"
#include "SerialSelector.h"
#include "SerialSelectorWidget.h"

namespace PokemonAutomation{


SerialSelector::~SerialSelector(){}
SerialSelector::SerialSelector(
    QString label,
    PABotBaseLevel minimum_pabotbase
)
    : m_label(std::move(label))
    , m_minimum_pabotbase(minimum_pabotbase)
{}
SerialSelector::SerialSelector(
    QString label,
    PABotBaseLevel minimum_pabotbase,
    const JsonValue& json
)
    : SerialSelector(std::move(label), minimum_pabotbase)
{
    load_json(json);
}
void SerialSelector::load_json(const JsonValue& json){
    const std::string* name = json.get_string();
    if (name == nullptr || name->empty()){
        return;
    }
    m_port = QSerialPortInfo(QString::fromStdString(*name));
}
JsonValue SerialSelector::to_json() const{
    return m_port->isNull() ? "" : m_port->portName().toStdString();
}

const QSerialPortInfo* SerialSelector::port() const{
    if (m_port->isNull()){
        return nullptr;
    }
    return m_port.get();
}

SerialSelectorWidget* SerialSelector::make_ui(QWidget& parent, LoggerQt& logger){
    return new SerialSelectorWidget(parent, *this, logger);
}






}




