/*  Egg Step Count Dropdown
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "PokemonSwSh_EggStepCount.h"

namespace PokemonAutomation{


const std::vector<uint16_t> STEP_COUNTS{
    1280,
    2560,
    3840,
    5120,
    6400,
    7680,
    8960,
    10240,
//    20480,
//    30720,
};



EggStepCountOption::EggStepCountOption()
    : m_option(
        "<b>Step Count:</b><br>Lookup the # of steps on Serebii.",
        {
            tostr_u_commas(STEP_COUNTS[0]),
            tostr_u_commas(STEP_COUNTS[1]),
            tostr_u_commas(STEP_COUNTS[2]),
            tostr_u_commas(STEP_COUNTS[3]),
            tostr_u_commas(STEP_COUNTS[4]),
            tostr_u_commas(STEP_COUNTS[5]),
            tostr_u_commas(STEP_COUNTS[6]),
            tostr_u_commas(STEP_COUNTS[7]),
        },
        3
    )
{}
void EggStepCountOption::load_json(const JsonValue& json){
    m_option.load_json(json);
}
JsonValue EggStepCountOption::to_json() const{
    return m_option.to_json();
}

void EggStepCountOption::restore_defaults(){
    m_option.restore_defaults();
}

ConfigWidget* EggStepCountOption::make_ui(QWidget& parent){
    return m_option.make_ui(parent);
}




}

