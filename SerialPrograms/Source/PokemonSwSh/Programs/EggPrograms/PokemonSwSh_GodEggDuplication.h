/*  God Egg Duplication
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_GodEggDuplication_H
#define PokemonAutomation_PokemonSwSh_GodEggDuplication_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh_EggHelpers.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class GodEggDuplication_Descriptor : public SingleSwitchProgramDescriptor{
public:
    GodEggDuplication_Descriptor();
};



class GodEggDuplication : public SingleSwitchProgramInstance{
public:
    GodEggDuplication();

    void collect_godegg(BotBaseContext& context, uint8_t party_slot) const;
    void run_program(Logger& logger, BotBaseContext& context, uint16_t attempts) const;
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
    SimpleIntegerOption<uint16_t> MAX_FETCH_ATTEMPTS;
    SimpleIntegerOption<uint8_t> PARTY_ROUND_ROBIN;
};


}
}
}
#endif
