/*  Money Farmer (Highlands)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/PokemonLA_MapDetector.h"
#include "PokemonLA/Inference/PokemonLA_DialogDetector.h"
#include "PokemonLA/Inference/PokemonLA_OverworldDetector.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include "PokemonLA/Programs/PokemonLA_EscapeFromAttack.h"
#include "PokemonLA_MoneyFarmerHighlands.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


MoneyFarmerHighlands_Descriptor::MoneyFarmerHighlands_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:MoneyFarmerHighlands",
        STRING_POKEMON + " LA", "Money Farmer (Highlands)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/MoneyFarmerHighlands.md",
        "Farm money off the Miss Fortune sisters in the Coronet Highlands.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


MoneyFarmerHighlands::MoneyFarmerHighlands(const MoneyFarmerHighlands_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , NOTIFICATION_STATUS("Status Update", true, false)
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(NOTIFICATIONS);
}



class MoneyFarmerHighlands::Stats : public StatsTracker{
public:
    Stats()
        : attempts(m_stats["Attempts"])
        , errors(m_stats["Errors"])
        , charm(m_stats["Charm"])
        , coin(m_stats["Coin"])
    {
        m_display_order.emplace_back("Attempts");
        m_display_order.emplace_back("Errors", true);
        m_display_order.emplace_back("Charm", true);
        m_display_order.emplace_back("Coin", true);
    }

    std::atomic<uint64_t>& attempts;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& charm;
    std::atomic<uint64_t>& coin;
};

std::unique_ptr<StatsTracker> MoneyFarmerHighlands::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



bool mash_A_until_end_of_battle(ProgramEnvironment& env, ConsoleHandle& console){
    OverworldDetector detector;
    int ret = run_until(
        env, console,
        [](const BotBaseContext& context){
            pbf_mash_button(context, BUTTON_A, 300 * TICKS_PER_SECOND);
        },
        { &detector }
    );
    if (ret < 0){
        console.log("Failed to return to overworld after 5 minutes.", COLOR_RED);
        return false;
    }
    console.log("Returned to overworld.");
    return true;
}
bool goto_camp_from_overworld(ProgramEnvironment& env, ConsoleHandle& console){
    while (true){
        EscapeFromAttack session(env, console);
        session.run_session();
        if (session.state() != UnderAttackState::SAFE){
            console.log("Unable to escape from being attacked.", COLOR_RED);
            return false;
        }

        //  Open the map.
        pbf_press_button(console, BUTTON_MINUS, 20, 30);
        {
            MapDetector detector;
            int ret = wait_until(
                env, console,
                std::chrono::seconds(5),
                { &detector }
            );
            if (ret < 0){
                console.log("Map not detected after 5 seconds.", COLOR_RED);
                return false;
            }
            console.log("Found map!");
            env.wait_for(std::chrono::milliseconds(500));
        }

        //  Try to fly back to camp.
        pbf_press_button(console, BUTTON_X, 20, 30);

        {
            ButtonDetector detector(
                console, console,
                ButtonType::ButtonA,
                {0.55, 0.40, 0.20, 0.40},
                std::chrono::milliseconds(200), true
            );
            int ret = wait_until(
                env, console,
                std::chrono::seconds(2),
                { &detector }
            );
            if (ret >= 0){
                console.log("Flying back to camp...");
                pbf_mash_button(console, BUTTON_A, 125);
                break;
            }
            console.log("Unable to fly. Are you under attack?", COLOR_RED);
        }

        pbf_mash_button(console, BUTTON_B, 125);
    }

    BlackScreenOverWatcher black_screen(COLOR_RED, {0.1, 0.1, 0.8, 0.6});
    int ret = wait_until(
        env, console,
        std::chrono::seconds(20),
        { &black_screen }
    );
    if (ret < 0){
        console.log("Failed to fly to camp after 5 seconds.", COLOR_RED);
        return false;
    }
    console.log("Arrived at camp...");
    env.wait_for(std::chrono::seconds(1));
    return true;
}





void MoneyFarmerHighlands::program(SingleSwitchProgramEnvironment& env){
    Stats& stats = env.stats<Stats>();


    //  Connect the controller.
    pbf_press_button(env.console, BUTTON_LCLICK, 5, 5);


    bool reset_required = false;

    while (true){
        env.update_stats();

        if (reset_required){
            pbf_press_button(env.console, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
            reset_game_from_home(env, env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
        }

        //  Go to Coronet Highlands Mountain camp.
        if (!goto_camp_from_jubilife(env, env.console, Camp::HIGHLANDS_MOUNTAIN)){
            stats.errors++;
            reset_required = true;
            continue;
        }

        stats.attempts++;

        //  Switch to Wrydeer.
        bool error = false;
        MountDetector mount_detector;
        while (true){
            MountState mount = mount_detector.detect(env.console.video().snapshot());
            if (mount == MountState::NOTHING){
                env.console.log("Unable to detect mount.", COLOR_RED);
                error = true;
                continue;
            }
            if (mount == MountState::WYRDEER_OFF){
                pbf_press_button(env.console, BUTTON_PLUS, 20, 105);
                break;
            }
            if (mount == MountState::WYRDEER_ON){
                pbf_wait(env.console, 5 * TICKS_PER_SECOND);
                break;
            }
            pbf_press_dpad(env.console, DPAD_LEFT, 20, 50);
            env.console.botbase().wait_for_all_requests();
        }
        if (error){
            stats.errors++;
            reset_required = true;
            continue;
        }


        bool success = false;


        env.console.log("Traveling to Charm's location...");
        {
            DialogDetector dialog_detector;
            //  TODO: Add shiny sound detector.
            int ret = run_until(
                env, env.console,
                [](const BotBaseContext& context){
                    pbf_move_left_joystick(context, 0, 212, 50, 0);
                    pbf_press_button(context, BUTTON_ZL, 50, 0);
                    pbf_press_button(context, BUTTON_B, 500, 125);

                    pbf_move_left_joystick(context, 224, 0, 50, 0);
                    pbf_press_button(context, BUTTON_ZL, 50, 0);
                    pbf_press_button(context, BUTTON_B, 400, 125);

                    pbf_move_left_joystick(context, 0, 128, 50, 0);
                    pbf_press_button(context, BUTTON_ZL, 50, 0);
                    pbf_press_button(context, BUTTON_B, 875, 0);
                },
                { &dialog_detector }
            );
            if (ret >= 0){
                env.console.log("Found Charm!", COLOR_BLUE);
                stats.charm++;
                if (mash_A_until_end_of_battle(env, env.console)){
                    env.console.log("Battle succeeded!", COLOR_BLUE);
                    success = true;
                }else{
                    env.console.log("Battle failed! Resetting...", COLOR_RED);
                    stats.errors++;
                    reset_required = true;
                    continue;
                }
            }
        }


#if 0
        env.console.log("Traveling to Coin's location...");
        {
            DialogDetector dialog_detector;

        }
#endif


        if (success){
            env.console.log("Returning to Jubilife...");
            if (!goto_camp_from_overworld(env, env.console)){
                reset_required = true;
                continue;
            }
            goto_professor(env.console, Camp::HIGHLANDS_HIGHLANDS);

            PA_THROW_StringException("Not yet implemented: Return to Jubilife.");

            reset_required = false;
        }else{
            env.console.log("Nothing found. Resetting...");
            reset_required = true;
        }
    }



}





}
}
}
