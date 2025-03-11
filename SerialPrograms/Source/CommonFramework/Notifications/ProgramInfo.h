/*  Program Information
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ProgramInfo_H
#define PokemonAutomation_ProgramInfo_H

#include <string>
#include "Common/Cpp/Time.h"

namespace PokemonAutomation{


struct ProgramInfo{
    std::string program_id;
    std::string program_name;
    WallClock start_time;

    ProgramInfo(
        const std::string& module = "",
        WallClock p_start_time = WallClock::min()
    )
        : program_name(module)
        , start_time(p_start_time)
    {}
    ProgramInfo(
        std::string p_program_id,
        std::string category, std::string display_name,
        WallClock p_start_time = WallClock::min()
    )
        : program_id(std::move(p_program_id))
        , program_name((category.empty() ? "" : category + ": ") + display_name)
        , start_time(p_start_time)
    {}
};



}
#endif
