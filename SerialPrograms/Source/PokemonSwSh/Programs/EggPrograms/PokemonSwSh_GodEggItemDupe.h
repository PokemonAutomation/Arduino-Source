/*  God Egg Item Duplication
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_GodEggItemDupe_H
#define PokemonAutomation_PokemonSwSh_GodEggItemDupe_H

#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh_EggHelpers.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class GodEggItemDupe_Descriptor : public SingleSwitchProgramDescriptor{
public:
    GodEggItemDupe_Descriptor();
};



class GodEggItemDupe : public SingleSwitchProgramInstance2{
public:
    GodEggItemDupe();

    void collect_godegg(BotBaseContext& context, uint8_t party_slot, bool map_to_pokemon, bool pokemon_to_map) const;
    void run_program(Logger& logger, BotBaseContext& context, uint16_t attempts) const;
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
    SimpleIntegerOption<uint16_t> MAX_FETCH_ATTEMPTS;
    SimpleIntegerOption<uint8_t> PARTY_ROUND_ROBIN;
    BooleanCheckBoxOption DETACH_BEFORE_RELEASE;
};


}
}
}
#endif
