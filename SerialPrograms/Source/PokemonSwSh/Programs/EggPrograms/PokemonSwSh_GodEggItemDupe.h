/*  God Egg Item Duplication
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_GodEggItemDupe_H
#define PokemonAutomation_PokemonSwSh_GodEggItemDupe_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh_EggHelpers.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class GodEggItemDupe_Descriptor : public SingleSwitchProgramDescriptor{
public:
    GodEggItemDupe_Descriptor();
};



class GodEggItemDupe : public SingleSwitchProgramInstance{
public:
    GodEggItemDupe();

    void collect_godegg(ProControllerContext& context, uint8_t party_slot, bool map_to_pokemon, bool pokemon_to_map) const;
    void run_program(Logger& logger, ProControllerContext& context, uint16_t attempts) const;
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    SimpleIntegerOption<uint16_t> MAX_FETCH_ATTEMPTS;
    SimpleIntegerOption<uint8_t> PARTY_ROUND_ROBIN;
    BooleanCheckBoxOption DETACH_BEFORE_RELEASE;

    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif
