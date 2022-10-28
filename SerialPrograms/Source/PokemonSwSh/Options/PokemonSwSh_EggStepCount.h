/*  Egg Step Count Dropdown
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EggStepCount_H
#define PokemonAutomation_PokemonSwSh_EggStepCount_H

#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{



class EggStepCountOption : public IntegerEnumDropdownOption{
public:
    EggStepCountOption()
        : IntegerEnumDropdownOption(
            "<b>Step Count:</b><br>Lookup the # of steps on Serebii.",
            {
                {1280,  "1280",     "1,280"},
                {2560,  "2560",     "2,560"},
                {3840,  "3840",     "3,840"},
                {5120,  "5120",     "5,120"},
                {6400,  "6400",     "6,400"},
                {7680,  "7680",     "7,680"},
                {8960,  "8960",     "8,960"},
                {10240, "10240",    "10,240"},
            },
            LockWhileRunning::LOCKED,
            5120
        )
    {}

    operator uint16_t() const{
        return (uint16_t)current_value();
    }
};



}
#endif

