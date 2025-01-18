/*  Serial Port (PABotBase) Globals
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Controllers_SerialPABotBase_Globals_H
#define PokemonAutomation_Controllers_SerialPABotBase_Globals_H

#include <stdint.h>
#include "Common/PokemonSwSh/PokemonProgramIDs.h"

namespace PokemonAutomation{


enum class AllowCommandsWhenRunning{
    DISABLE_COMMANDS,
    ENABLE_COMMANDS,
};


enum class PABotBaseLevel{
    NOT_PABOTBASE = 0,
    PABOTBASE_12KB = 1,
    PABOTBASE_31KB = 2,
};


inline PABotBaseLevel program_id_to_botbase_level(uint8_t id){
    switch (id){
    case PABB_PID_PABOTBASE_12KB:
        return PABotBaseLevel::PABOTBASE_12KB;
    case PABB_PID_PABOTBASE_31KB:
        return PABotBaseLevel::PABOTBASE_31KB;
    }
    return PABotBaseLevel::NOT_PABOTBASE;
}



}
#endif
