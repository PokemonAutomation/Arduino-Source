/*  Program IDs for Serial Communication
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#ifndef PokemonAutomation_ProgramIDs_H
#define PokemonAutomation_ProgramIDs_H

#include <stdint.h>



#define PABB_PID_UNSPECIFIED                                0x00
#define PABB_PID_PABOTBASE_12KB                             0x08
#define PABB_PID_PABOTBASE_31KB                             0x09



#ifdef __cplusplus

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

#endif



#endif
