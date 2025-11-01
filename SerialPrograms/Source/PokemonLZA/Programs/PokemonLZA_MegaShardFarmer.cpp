/*  Mega Shard Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA_MegaShardFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;


MegaShardFarmer_Descriptor::MegaShardFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:MegaShardFarmer",
        STRING_POKEMON + " LZA", "Mega Shard Farmer",
        "Programs/PokemonLZA/MegaShardFarmer.html",
        "Farm the mega shards behind Restaurant Le Yeah.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}
class MegaShardFarmer_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : rounds(m_stats["Rounds"])
    {
        m_display_order.emplace_back("Rounds");
    }

    std::atomic<uint64_t>& rounds;
};
std::unique_ptr<StatsTracker> MegaShardFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



MegaShardFarmer::MegaShardFarmer(){}

void MegaShardFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    MegaShardFarmer_Descriptor::Stats& stats = env.current_stats<MegaShardFarmer_Descriptor::Stats>();

    while (true){
        env.update_stats();

        //  Travel to alley with logs of mega shards.
        pbf_move_left_joystick(context, 0, 96, 400ms, 400ms);
        pbf_press_button(context, BUTTON_L, 160ms, 440ms);
        pbf_move_left_joystick(context, 128, 0, 3000ms, 400ms);
        pbf_move_left_joystick(context, 255, 128, 400ms, 400ms);
        pbf_press_button(context, BUTTON_L, 160ms, 440ms);
        pbf_move_left_joystick(context, 128, 0, 2500ms, 400ms);
        pbf_move_left_joystick(context, 255, 64, 400ms, 400ms);
        pbf_press_button(context, BUTTON_L, 160ms, 440ms);
        pbf_move_left_joystick(context, 128, 0, 2000ms, 0ms);
        pbf_move_left_joystick(context, 160, 0, 1000ms, 0ms);
        pbf_move_left_joystick(context, 128, 0, 2000ms, 0ms);
        pbf_move_left_joystick(context, 160, 0, 1500ms, 1000ms);

        //  Move camera up.
        pbf_move_right_joystick(context, 128, 0, 360ms, 0ms);

        //  Bring out a Pokemon.
        pbf_press_dpad(context, DPAD_UP, 480ms, 0ms);

        //  Break all the shards.
        for (int i = 0; i < 5; i++){
            for (int c = 0; c < 5; c++){
                ssf_press_button(context, BUTTON_ZL, 240ms, 1000ms, 240ms);
                pbf_press_button(context, BUTTON_B, 160ms, 1840ms);
                pbf_move_right_joystick(context, 255, 96, 240ms, 0ms);
            }
            for (int c = 0; c < 5; c++){
                ssf_press_button(context, BUTTON_ZL, 240ms, 1000ms, 240ms);
                pbf_press_button(context, BUTTON_B, 160ms, 1840ms);
                pbf_move_right_joystick(context, 255, 144, 240ms, 0ms);
            }
        }

        fly_back(env, context);

        stats.rounds++;
    }

}
void MegaShardFarmer::fly_back(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    pbf_press_button(context, BUTTON_PLUS, 240ms, 1000ms);
    pbf_move_left_joystick(context, 0, 128, 80ms, 500ms);
    pbf_mash_button(context, BUTTON_A, 5000ms);

}


}
}
}
