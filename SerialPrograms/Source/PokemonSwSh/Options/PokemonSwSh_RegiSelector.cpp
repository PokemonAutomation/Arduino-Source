/*  Regi Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "PokemonSwSh_RegiSelector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{




RegiSelectorOption::RegiSelectorOption()
    : m_option(
        "<b>Name of Regi:</b>",
        {
            "Regirock",
            "Regice",
            "Registeel",
            "Regieleki",
            "Regidrago",
        },
        2
    )
{}
void RegiSelectorOption::load_json(const JsonValue& json){
    m_option.load_json(json);
}
JsonValue RegiSelectorOption::to_json() const{
    return m_option.to_json();
}

void RegiSelectorOption::restore_defaults(){
    m_option.restore_defaults();
}

ConfigWidget* RegiSelectorOption::make_ui(QWidget& parent){
    return m_option.make_ui(parent);
}




}
}
}

