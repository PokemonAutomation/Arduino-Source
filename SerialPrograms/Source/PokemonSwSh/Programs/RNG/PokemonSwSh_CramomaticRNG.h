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
#include "PokemonSwSh/Options/PokemonSwSh_CramomaticTable.h"
#include "PokemonSwSh/Programs/RNG/PokemonSwSh_Xoroshiro128Plus.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonSwSh {

class CramomaticRNG_Descriptor : public SingleSwitchProgramDescriptor {
public:
    CramomaticRNG_Descriptor();
};

struct CramomaticTarget {
    CramomaticBallType ball_type;
    bool is_bonus;
    size_t needed_advances;
};

class CramomaticRNG : public SingleSwitchProgramInstance {
public:
    CramomaticRNG();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:

    StartInGripOrGameOption START_LOCATION;
    SimpleIntegerOption<uint32_t> NUM_APRICORN_ONE;
    SimpleIntegerOption<uint32_t> NUM_APRICORN_TWO;
    SimpleIntegerOption<uint32_t> NUM_NPCS;
    CramomaticTable BALL_TABLE;


    SectionDividerOption m_advanced_options;
    SimpleIntegerOption<uint16_t> MAX_UNKNOWN_ADVANCES;
    BooleanCheckBoxOption SAVE_SCREENSHOTS;
    BooleanCheckBoxOption LOG_VALUES;

    void navigate_to_party(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    CramomaticTarget needed_advances(SingleSwitchProgramEnvironment& env, Xoroshiro128PlusState state, std::vector<CramomaticSelection> wanted_balls);
    void leave_to_overworld_and_interact(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    void choose_apricorn(SingleSwitchProgramEnvironment& env, BotBaseContext& context, bool sport);
    bool receive_ball(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
};




}
}
}
#endif



