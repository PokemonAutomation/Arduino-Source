/*  Serial Connection Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonValue>
#include "SerialSelector.h"
#include "SerialSelectorWidget.h"

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

SerialSelectorWidget* SerialSelector::make_ui(QWidget& parent, Logger& logger){
    return new SerialSelectorWidget(parent, *this, logger);
}






}




