/*  Berry Farmer 2
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BerryFarmer2_H
#define PokemonAutomation_PokemonSwSh_BerryFarmer2_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class BerryFarmer2_Descriptor : public SingleSwitchProgramDescriptor {
public:
    BerryFarmer2_Descriptor();
};



class BerryFarmer2 : public SingleSwitchProgramInstance{
public:
    BerryFarmer2();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

    enum Rustling
    {
        No,
        Slow,
        Fast,
        Battle
    };

private:
    Rustling check_rustling(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    uint16_t do_secondary_attempts(SingleSwitchProgramEnvironment& env, BotBaseContext& context, Rustling last_rustling);

private:
    StaticTextOption REQUIRES_AUDIO;
    StartInGripOrGameOption START_LOCATION;

    SimpleIntegerOption<uint32_t> FETCH_ATTEMPTS;
    SimpleIntegerOption<uint16_t> SAVE_ITERATIONS;

    SectionDividerOption m_advanced_options;
    TimeExpressionOption<uint16_t> EXIT_BATTLE_TIMEOUT;
    SimpleIntegerOption<uint16_t> START_BATTLE_TIMEOUT;
    SimpleIntegerOption<uint16_t> RUSTLING_INTERVAL;
    TimeExpressionOption<uint16_t> RUSTLING_TIMEOUT;
    TimeExpressionOption<uint16_t> SECONDARY_ATTEMPT_MASH_TIME;
};


}
}
}
#endif



