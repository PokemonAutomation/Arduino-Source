/*  Overworld Trigger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_OverworldTrigger_H
#define PokemonAutomation_PokemonBDSP_OverworldTrigger_H

#include "CommonFramework/Options/BatchOption.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "CommonFramework/Tools/ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

// Program component to trigger an overworld encounter.
class OverworldTrigger : public GroupOption{
public:
    OverworldTrigger();

    // Move character back and forth or use Sweet Scent to trigger an encounter.
    // Return true if the start of a battle is detected.
    // Return false if an unexpected battle happens where the battle menu is detected but not
    //   the starting animation.
    // Throw exception if inference times out after Sweet Scent is used.
    bool find_encounter(SingleSwitchProgramEnvironment& env) const;

private:
    // Move character up and down or left and right once.
    void run_trigger(const BotBaseContext& context) const;

public:
    EnumDropdownOption TRIGGER_METHOD;
    TimeExpressionOption<uint16_t> MOVE_DURATION;
    EnumDropdownOption SWEET_SCENT_POKEMON_LOCATION;
};



}
}
}
#endif
