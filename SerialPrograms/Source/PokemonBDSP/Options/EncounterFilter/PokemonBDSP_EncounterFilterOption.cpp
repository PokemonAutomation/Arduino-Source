/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "Common/Cpp/Json/JsonValue.h"
//#include "Common/Cpp/Json/JsonObject.h"
//#include "CommonFramework/Globals.h"
//#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP_EncounterFilterEnums.h"
#include "PokemonBDSP_EncounterFilterOption.h"
//#include "PokemonBDSP_EncounterFilterWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
//    using namespace Pokemon;



#if 0
EncounterFilterOption::EncounterFilterOption(bool enable_overrides)
    : m_enable_overrides(enable_overrides)
    , m_shiny_filter_default(ShinyFilter::SHINY)
    , m_shiny_filter_current(m_shiny_filter_default)
{}
void EncounterFilterOption::load_json(const JsonValue& json){
    using namespace Pokemon;

    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }

    const std::string* str = obj->get_string("ShinyFilter");
    if (str != nullptr){
        auto iter = ShinyFilter_MAP.find(*str);
        if (iter != ShinyFilter_MAP.end()){
            m_shiny_filter_current.store(iter->second, std::memory_order_release);
        }
    }

    if (m_enable_overrides){
        const JsonValue* array = obj->get_value("Overrides");
        if (array != nullptr){
            m_table.load_json(*array);
        }
    }
}
JsonValue EncounterFilterOption::to_json() const{
    JsonObject obj;
    obj["ShinyFilter"] = ShinyFilter_NAMES[(size_t)m_shiny_filter_current.load(std::memory_order_acquire)];

    if (m_enable_overrides){
        obj["Overrides"] = m_table.to_json();
    }

    return obj;
}
void EncounterFilterOption::restore_defaults(){
    m_shiny_filter_current.store(m_shiny_filter_default, std::memory_order_release);
    m_table.restore_defaults();
}
ConfigWidget* EncounterFilterOption::make_QtWidget(QWidget& parent){
    return new EncounterFilterWidget(parent, *this);
}
#endif


EncounterFilterOption2::EncounterFilterOption2(bool enable_overrides)
    : BatchOption(LockMode::UNLOCK_WHILE_RUNNING)
    , SHINY_FILTER(
        "<b>Stop on:</b>",
        ShinyFilter_Database(),
        LockMode::UNLOCK_WHILE_RUNNING,
        ShinyFilter::SHINY
    )
{
    PA_ADD_OPTION(SHINY_FILTER);
    if (enable_overrides){
        PA_ADD_OPTION(FILTER_TABLE);
    }
}





}
}
}
