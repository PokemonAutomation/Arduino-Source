/*  Overworld Trigger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_OverworldTrigger_H
#define PokemonAutomation_PokemonBDSP_OverworldTrigger_H

#include "CommonFramework/Options/BatchOption.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "CommonFramework/Tools/ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class OverworldTrigger : public GroupOption{
public:
    OverworldTrigger();

    void run_trigger(const BotBaseContext& context) const;

public:
    EnumDropdownOption TRIGGER_METHOD;
    TimeExpressionOption<uint16_t> MOVE_DURATION;
};



}
}
}
#endif
