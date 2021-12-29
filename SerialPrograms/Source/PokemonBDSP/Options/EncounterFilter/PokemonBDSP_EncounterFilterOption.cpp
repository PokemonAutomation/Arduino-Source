/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Globals.h"
#include "PokemonBDSP_EncounterFilterEnums.h"
#include "PokemonBDSP_EncounterFilterOption.h"
#include "PokemonBDSP_EncounterFilterWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{




EncounterFilterOption::EncounterFilterOption(bool enable_overrides, bool allow_autocatch)
    : m_label("<b>Encounter Filter:</b>")
    , m_enable_overrides(enable_overrides)
    , m_allow_autocatch(allow_autocatch)
    , m_shiny_filter_default(ShinyFilter::SHINY)
    , m_shiny_filter_current(m_shiny_filter_default)
    , m_factory(allow_autocatch)
    , m_table(
        "<b>Overrides:</b><br>"
        "The game language must be properly set to read " + STRING_POKEMON + " names. "
        "If multiple overrides apply and are conflicting, the program will stop.",
        m_factory, false
    )
{}
std::vector<EncounterFilterOverride> EncounterFilterOption::overrides() const{
    std::vector<EncounterFilterOverride> ret;
    for (size_t c = 0; c < m_table.size(); c++){
        ret.emplace_back(static_cast<const EncounterFilterOverride&>(m_table[c]));
    }
    return ret;
}
void EncounterFilterOption::load_json(const QJsonValue& json){
    using namespace Pokemon;

    QJsonObject obj = json.toObject();

    QString shiny_filter;
    if (json_get_string(shiny_filter, obj, "ShinyFilter")){
        auto iter = ShinyFilter_MAP.find(shiny_filter);
        if (iter != ShinyFilter_MAP.end()){
            m_shiny_filter_current = iter->second;
        }
    }

    if (m_enable_overrides){
        m_table.load_json(json_get_array_nothrow(obj, "Overrides"));
    }
}
QJsonValue EncounterFilterOption::to_json() const{
    QJsonObject obj;
    obj.insert("ShinyFilter", ShinyFilter_NAMES[(size_t)m_shiny_filter_current]);

    if (m_enable_overrides){
        obj.insert("Overrides", m_table.to_json());
    }

    return obj;
}
void EncounterFilterOption::restore_defaults(){
    m_shiny_filter_current = m_shiny_filter_default;
    m_table.restore_defaults();
}
ConfigWidget* EncounterFilterOption::make_ui(QWidget& parent){
    return new EncounterFilterWidget(parent, *this);
}




}
}
}
