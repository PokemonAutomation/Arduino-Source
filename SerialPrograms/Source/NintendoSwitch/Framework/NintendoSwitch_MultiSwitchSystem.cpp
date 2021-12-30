/*  Switch System (4 Switches)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Qt/QtJsonTools.h"
#include "NintendoSwitch_MultiSwitchSystem.h"
#include "NintendoSwitch_MultiSwitchSystemWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


MultiSwitchSystemFactory::MultiSwitchSystemFactory(
    PABotBaseLevel min_pabotbase, FeedbackType feedback,
    size_t min_switches,
    size_t max_switches,
    size_t switches
)
    : SwitchSetupFactory(min_pabotbase, feedback)
    , m_min_switches(std::max(min_switches, (size_t)1))
    , m_max_switches(std::min(max_switches, (size_t)MAX_SWITCHES))
    , m_active_switches(0)
{
    switches = std::max(switches, m_min_switches);
    switches = std::min(switches, m_max_switches);
    resize(switches);
}
MultiSwitchSystemFactory::MultiSwitchSystemFactory(
    PABotBaseLevel min_pabotbase, FeedbackType feedback,
    size_t min_switches,
    size_t max_switches,
    const QJsonValue& json
)
    : SwitchSetupFactory(min_pabotbase, feedback)
    , m_min_switches(std::max(min_switches, (size_t)1))
    , m_max_switches(std::min(max_switches, (size_t)MAX_SWITCHES))
    , m_active_switches(0)
{
    MultiSwitchSystemFactory::load_json(json);
    if (m_switches.size() < m_min_switches){
        resize(m_min_switches);
    }
}
void MultiSwitchSystemFactory::load_json(const QJsonValue& json){
    QJsonObject obj = json.toObject();
    QJsonArray array = json_get_array_nothrow(obj, "DeviceList");
    if (array.size() == 0 || (size_t)array.size() > MAX_SWITCHES){
        return;
    }
    m_switches.clear();
    for (int c = 0; c < array.size(); c++){
        m_switches.emplace_back(
            new SwitchSystemFactory(
                QString("Switch ") + QString::number(c),
                "Switch " + std::to_string(c),
                m_min_pabotbase, m_feedback,
                array[c]
            )
        );
    }
    json_get_int(m_active_switches, obj, "ActiveDevices", (int)m_min_switches, (int)m_max_switches);
}
QJsonValue MultiSwitchSystemFactory::to_json() const{
    QJsonObject obj;
    obj.insert("ActiveDevices", QJsonValue((int)m_active_switches));
    QJsonArray array;
    for (const auto& item : m_switches){
        array.push_back(item->to_json());
    }
    obj.insert("DeviceList", array);
    return obj;
}
void MultiSwitchSystemFactory::resize(size_t count){
    while (m_switches.size() < count){
        m_switches.emplace_back(
            new SwitchSystemFactory(
                QString("Switch ") + QString::number(m_switches.size()),
                "Switch " + std::to_string(m_switches.size()),
                m_min_pabotbase, m_feedback
            )
        );
    }
    m_active_switches = count;
}

const QSerialPortInfo* MultiSwitchSystemFactory::port(size_t index) const{
    return m_switches[index]->port();
}
//const QCameraInfo* MultiSwitchSystemFactory::camera(size_t index) const{
//    return m_switches[index]->camera();
//}

SwitchSetupWidget* MultiSwitchSystemFactory::make_ui(QWidget& parent, Logger& logger, uint64_t program_id){
    return new MultiSwitchSystemWidget(parent, *this, logger, program_id);
}




}
}





