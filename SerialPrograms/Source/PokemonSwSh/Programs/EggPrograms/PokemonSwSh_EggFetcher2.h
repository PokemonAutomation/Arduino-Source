/*  Egg Fetcher 2
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EggFetcher2_H
#define PokemonAutomation_PokemonSwSh_EggFetcher2_H

#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh_EggHelpers.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class EggFetcher2_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    EggFetcher2_Descriptor();
};



class EggFetcher2 : public SingleSwitchProgramInstance{
public:
    EggFetcher2(const EggFetcher2_Descriptor& descriptor);

    void run_eggfetcher(Logger& logger, BotBaseContext& context, bool deposit_automatically, uint16_t attempts) const;
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
    SimpleIntegerOption<uint16_t> MAX_FETCH_ATTEMPTS;
};



}
}
}
#endif
