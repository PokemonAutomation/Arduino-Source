/*  Mega Shard Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
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
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
class MegaShardFarmer_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : rounds(m_stats["Rounds"])
        , day_changes(m_stats["Day/Night Changes"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Rounds");
        m_display_order.emplace_back("Day/Night Changes");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& rounds;
    std::atomic<uint64_t>& day_changes;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> MegaShardFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



MegaShardFarmer::MegaShardFarmer()
    : SKIP_SHARDS(
        "<b>Skip getting Shards (for testing purposes):</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
{
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(SKIP_SHARDS);
    }
}

void MegaShardFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    MegaShardFarmer_Descriptor::Stats& stats = env.current_stats<MegaShardFarmer_Descriptor::Stats>();


    while (true){
        env.update_stats();

        fly_back(env, context);
        context.wait_for_all_requests();

        BlackScreenOverWatcher black_screen(
            COLOR_BLUE,
            {0.1, 0.1, 0.8, 0.8},
            100, 10,
            500ms
        );

        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                //  Travel to alley with logs of mega shards.
                pbf_move_left_joystick(context, {-1, +0.25}, 400ms, 400ms);
                pbf_press_button(context, BUTTON_L, 160ms, 440ms);
                pbf_move_left_joystick(context, {0, +1}, 3000ms, 400ms);
                pbf_move_left_joystick(context, {+1, 0}, 400ms, 400ms);
                pbf_press_button(context, BUTTON_L, 160ms, 440ms);
                pbf_move_left_joystick(context, {0, +1}, 2500ms, 400ms);
                pbf_move_left_joystick(context, {+1, +0.5}, 400ms, 400ms);
                pbf_press_button(context, BUTTON_L, 160ms, 440ms);
                pbf_move_left_joystick(context, {0, +1}, 2000ms, 0ms);
                pbf_move_left_joystick(context, {+0.252, +1}, 1000ms, 0ms);
                pbf_move_left_joystick(context, {0, +1}, 2000ms, 0ms);
                pbf_move_left_joystick(context, {+0.252, +1}, 1500ms, 1000ms);

                //  Move camera up.
                pbf_move_right_joystick(context, {0, +1}, 800ms, 0ms);

                //  Bring out a Pokemon.
                pbf_press_dpad(context, DPAD_UP, 1000ms, 0ms);

                if (!SKIP_SHARDS){
                    //  Break all the shards.
                    uint8_t x = stats.rounds % 2 == 0 ? 128 - 32 : 128 + 32;
                    ssf_press_right_joystick_old(context, x, 128, 0ms, 40000ms, 0ms);
                    for (int c = 0; c < 40; c++){
                        ssf_press_button(context, BUTTON_ZL, 240ms, 400ms, 80ms);
                        pbf_press_button(context, BUTTON_B, 160ms, 520ms);
                    }
                }
            },
            {{black_screen}}
        );

        if (ret == 0){
            env.log("Detected day/night change.");
            stats.day_changes++;
            context.wait_for(15000ms);
        }

        stats.rounds++;
    }

}
void MegaShardFarmer::fly_back(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    while (true){
        const bool zoom_to_max = false;
        open_map(env.console, context, zoom_to_max);

        //  Middle Zoom
        pbf_move_right_joystick(context, {0, +1}, 80ms, 80ms);
        pbf_move_right_joystick(context, {0, +1}, 80ms, 80ms);
        pbf_move_right_joystick(context, {0, -1}, 80ms, 80ms);

        //  Tap the stick to lock on to Le Yeah if you're already on top of it.
        pbf_move_left_joystick(context, {0, -0.5}, 40ms, 120ms);
        pbf_move_left_joystick(context, {0, +0.5}, 40ms, 500ms);

        if (fly_from_map(env.console, context) == FastTravelState::SUCCESS){
            return;
        }else{
            MegaShardFarmer_Descriptor::Stats& stats = env.current_stats<MegaShardFarmer_Descriptor::Stats>();
            stats.errors++;
            pbf_mash_button(context, BUTTON_B, 5000ms);
        }
    }
}


}
}
}
