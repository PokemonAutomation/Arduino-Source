/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh_EncounterFilterEnums.h"
#include "PokemonSwSh_EncounterFilterOption.h"
#include "PokemonSwSh_EncounterFilterWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{




#if 0
EncounterFilterOption::EncounterFilterOption(bool rare_stars, bool enable_overrides)
    : m_rare_stars(rare_stars)
    , m_enable_overrides(enable_overrides)
    , m_shiny_filter_default(ShinyFilter::ANY_SHINY)
    , m_shiny_filter_current(m_shiny_filter_default)
    , m_table(rare_stars)
{}
void EncounterFilterOption::load_json(const JsonValue& json){
    using namespace Pokemon;

    const JsonObject* obj = json.to_object();

    const std::string* str = obj->get_string("ShinyFilter");
    if (str != nullptr){
        auto iter = ShinyFilter_MAP.find(*str);
        if (iter != ShinyFilter_MAP.end()){
            m_shiny_filter_current = iter->second;
        }
    }

    if (m_enable_overrides){
        const JsonValue* value = obj->get_value("Overrides");
        if (value != nullptr){
            m_table.load_json(*value);
        }
    }
}
JsonValue EncounterFilterOption::to_json() const{
    JsonObject obj;
    obj["ShinyFilter"] = ShinyFilter_NAMES[(size_t)m_shiny_filter_current.load(std::memory_order_relaxed)];
    if (m_enable_overrides){
        obj["Overrides"] = m_table.to_json();
    }
    return obj;
}
void EncounterFilterOption::restore_defaults(){
    m_shiny_filter_current = m_shiny_filter_default;
    m_table.restore_defaults();
}
ConfigWidget* EncounterFilterOption::make_QtWidget(QWidget& parent){
    return new EncounterFilterWidget(parent, *this);
}
#endif





EncounterFilterOption2::EncounterFilterOption2(bool rare_stars, bool enable_overrides)
    : BatchOption(LockMode::UNLOCK_WHILE_RUNNING)
    , SHINY_FILTER(
        "<b>Stop on:</b>",
        rare_stars ? ShinyFilter_StopRareStars_Database() : ShinyFilter_Normal_Database(),
        LockMode::UNLOCK_WHILE_RUNNING,
        ShinyFilter::ANY_SHINY
    )
    , FILTER_TABLE(rare_stars)
{
    PA_ADD_OPTION(SHINY_FILTER);
    if (enable_overrides){
        PA_ADD_OPTION(FILTER_TABLE);
    }
}





















}
}
}
