/*  PokeJobs Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_PokeJobsFarmer_H
#define PokemonAutomation_PokemonSwSh_PokeJobsFarmer_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class PokeJobsFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    PokeJobsFarmer_Descriptor();
};

class PokeJobsFarmer : public SingleSwitchProgramInstance{
public:
    PokeJobsFarmer();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;

    SimpleIntegerOption<uint32_t> SKIPS;
    SimpleIntegerOption<uint32_t> CONCURRENCY;
    SimpleIntegerOption<uint16_t> MENU_INDEX;

    SectionDividerOption m_advanced_options;

    MillisecondsOption MASH_B_DURATION0;

    EventNotificationsOption NOTIFICATIONS;
};



}
}
}
#endif
