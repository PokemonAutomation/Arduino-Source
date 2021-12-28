/*  Egg Fetcher Multiple
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EggFetcherMultiple_H
#define PokemonAutomation_PokemonSwSh_EggFetcherMultiple_H

#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh_EggHelpers.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class EggFetcherMultiple_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    EggFetcherMultiple_Descriptor();
};



class EggFetcherMultiple : public SingleSwitchProgramInstance{
public:
    EggFetcherMultiple(const EggFetcherMultiple_Descriptor& descriptor);

    void run_eggfetcher(SingleSwitchProgramEnvironment& env, bool deposit_automatically, uint16_t attempts) const;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
    SimpleIntegerOption<uint16_t> POKEMON_SPECIES_COUNT;
    SimpleIntegerOption<uint16_t> MAX_FETCH_ATTEMPTS_PER_SPECIES;
};



}
}
}
#endif
