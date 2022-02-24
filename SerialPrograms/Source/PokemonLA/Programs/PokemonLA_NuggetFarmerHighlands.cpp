/*  Money Farmer (Highlands)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
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
#include "PokemonLA_NuggetFarmerHighlands.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


NuggetFarmerHighlands_Descriptor::NuggetFarmerHighlands_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:NuggetFarmerHighlands",
        STRING_POKEMON + " LA", "Nugget Farmer (Highlands)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/NuggetFarmerHighlands.md",
        "Farm nuggets off the Miss Fortune sisters in the Coronet Highlands.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


MoneyFarmerHighlands::MoneyFarmerHighlands(const NuggetFarmerHighlands_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
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
    OverworldDetector detector(console, console);
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





void MoneyFarmerHighlands::program(SingleSwitchProgramEnvironment& env){
    Stats& stats = env.stats<Stats>();


    //  Connect the controller.
    pbf_press_button(env.console, BUTTON_LCLICK, 5, 5);


    bool reset_required = false;

    while (true){
        env.update_stats();
        send_program_status_notification(
            env.logger(), NOTIFICATION_STATUS,
            env.program_info(),
            "",
            stats.to_str()
        );

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
            DialogDetector dialog_detector(env.console, env.console);
            //  TODO: Add shiny sound detector.
            int ret = run_until(
                env, env.console,
                [](const BotBaseContext& context){
                    pbf_move_left_joystick(context, 0, 212, 50, 0);
                    pbf_press_button(context, BUTTON_B, 500, 80);

                    pbf_move_left_joystick(context, 224, 0, 50, 0);
                    pbf_press_button(context, BUTTON_B, 350, 80);

                    pbf_move_left_joystick(context, 0, 64, 50, 0);
                    pbf_press_button(context, BUTTON_B, 250, 80);

                    pbf_move_left_joystick(context, 0, 48, 50, 0);
                    pbf_press_button(context, BUTTON_B, 270, 0);

                    pbf_move_left_joystick(context, 0, 255, 50, 0);
                    pbf_press_button(context, BUTTON_B, 150, 250);

//                    pbf_move_right_joystick(context, 0, 128, 200, 125);

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
                stats.errors++;
                reset_required = true;
                continue;
            }
            goto_professor(env.console, Camp::HIGHLANDS_HIGHLANDS);
            from_professor_return_to_jubilife(env, env.console);
            if (!save_game_from_overworld(env, env.console)){
                stats.errors++;
                reset_required = true;
            }else{
                reset_required = false;
            }
            env.console.botbase().wait_for_all_requests();
        }else{
            env.console.log("Nothing found. Resetting...");
//            if (!goto_camp_from_overworld(env, env.console)){
//                reset_required = true;
//                continue;
//            }
            reset_required = true;
        }
    }



}





}
}
}
