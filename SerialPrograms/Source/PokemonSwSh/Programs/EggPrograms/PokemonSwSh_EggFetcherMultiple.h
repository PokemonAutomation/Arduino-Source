/*  Egg Fetcher Multiple
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EggFetcherMultiple_H
#define PokemonAutomation_PokemonSwSh_EggFetcherMultiple_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
//#include "PokemonSwSh_EggHelpers.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class EggFetcherMultiple_Descriptor : public SingleSwitchProgramDescriptor{
public:
    EggFetcherMultiple_Descriptor();
};



class EggFetcherMultiple : public SingleSwitchProgramInstance{
public:
    EggFetcherMultiple();

    void run_eggfetcher(Logger& logger, ProControllerContext& context, bool deposit_automatically, uint16_t attempts) const;
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    SimpleIntegerOption<uint16_t> POKEMON_SPECIES_COUNT;
    SimpleIntegerOption<uint16_t> MAX_FETCH_ATTEMPTS_PER_SPECIES;

    EventNotificationsOption NOTIFICATIONS;
};



}
}
}
#endif
