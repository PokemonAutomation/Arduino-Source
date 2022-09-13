/*  Cram-o-matic RNG Manipulation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_CramomaticRNG_H
#define PokemonAutomation_PokemonSwSh_CramomaticRNG_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Programs/RNG/PokemonSwSh_Xoroshiro128Plus.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonSwSh {

class CramomaticRNG_Descriptor : public SingleSwitchProgramDescriptor {
public:
    CramomaticRNG_Descriptor();
};



class CramomaticRNG : public SingleSwitchProgramInstance {
public:
    CramomaticRNG();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    enum BallType
    {
        Poke,
        Great,
        Shop1,
        Shop2,
        Apricorn,
        Safari,
        Sport
    };

    StartInGripOrGameOption START_LOCATION;
    SimpleIntegerOption<uint32_t> NUM_ITERATIONS;
    SimpleIntegerOption<uint32_t> NUM_NPCS;
    EnumDropdownOption<BallType> BALL_TYPE;


    SectionDividerOption m_advanced_options;
    SimpleIntegerOption<uint16_t> MAX_UNKNOWN_ADVANCES;
    BooleanCheckBoxOption ONLY_BONUS;
    BooleanCheckBoxOption SAVE_SCREENSHOTS;
    BooleanCheckBoxOption LOG_VALUES;

    void navigate_to_party(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    size_t needed_advances(SingleSwitchProgramEnvironment& env, Xoroshiro128PlusState state, BallType wanted_type);
    void leave_to_overworld_and_interact(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    void choose_apricorn(SingleSwitchProgramEnvironment& env, BotBaseContext& context, bool sport);
    bool receive_ball(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
};




}
}
}
#endif



