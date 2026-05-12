/*  Max Lair Options
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Options_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Options_H

#include <chrono>
#include "Common/Compiler.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_Stats.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


extern const std::string MODULE_NAME;
extern const std::chrono::milliseconds INFERENCE_RATE;




class HostingSwitch : public IntegerEnumDropdownOption{
public:
    HostingSwitch();
    using ConfigOption::check_validity;
    std::string check_validity(size_t consoles) const;
};



class BossSlotOption : public IntegerEnumDropdownOption{
public:
    BossSlotOption();
};




enum class CaughtScreenAction{
    STOP_PROGRAM,
    TAKE_NON_BOSS_SHINY_AND_CONTINUE,
    RESET,
};
class EndBattleDecider{
public:
    virtual const std::string& normal_ball(
        size_t console_index
    ) const = 0;
    virtual const std::string& boss_ball(
        size_t console_index, const std::string& boss_slug
    ) const = 0;
    virtual CaughtScreenAction end_adventure_action(
        size_t console_index, const std::string& boss_slug,
        const PathStats& path_stats,
        bool any_shiny, bool boss_is_shiny
    ) const = 0;
};








}
}
}
}
#endif
