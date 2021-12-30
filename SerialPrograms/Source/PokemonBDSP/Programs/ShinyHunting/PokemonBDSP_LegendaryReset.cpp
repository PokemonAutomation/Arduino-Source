/*  Shiny Hunt - Legendary Reset
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_StartBattleDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/ShinyDetection/PokemonBDSP_ShinyEncounterDetector.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameEntry.h"
#include "PokemonBDSP/Programs/PokemonBDSP_EncounterHandler.h"
#include "PokemonBDSP_LegendaryReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


LegendaryReset_Descriptor::LegendaryReset_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonBDSP:LegendaryReset",
        STRING_POKEMON + " BDSP", "Legendary Reset",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/LegendaryReset.md",
        "Shiny hunt a standing legendary " + STRING_POKEMON + ".",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


LegendaryReset::LegendaryReset(const LegendaryReset_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , WALK_UP("<b>Walk Up:</b><br>Walk up while mashing A to trigger encounter.", false)
    , ENCOUNTER_BOT_OPTIONS(false, false)
    , NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true)
    , NOTIFICATIONS({
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_NONSHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_SHINY,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);

    PA_ADD_OPTION(LANGUAGE);

    PA_ADD_OPTION(WALK_UP);

    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);
}


std::unique_ptr<StatsTracker> LegendaryReset::make_stats() const{
    return std::unique_ptr<StatsTracker>(new PokemonSwSh::ShinyHuntTracker(false));
}


void LegendaryReset::program(SingleSwitchProgramEnvironment& env){
    PokemonSwSh::ShinyHuntTracker& stats = env.stats<PokemonSwSh::ShinyHuntTracker>();

    StandardEncounterHandler handler(
        env, env.console,
        LANGUAGE,
        ENCOUNTER_BOT_OPTIONS,
        stats
    );

    //  Connect the controller.
    pbf_press_button(env.console, BUTTON_B, 5, 5);

    bool reset = false;
    while (true){
        env.update_stats();

        if (reset){
            pbf_press_button(env.console, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY);
            if (!reset_game_from_home(env, env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST)){
                stats.add_error();
                continue;
            }
        }
        reset = true;

        StartBattleDetector start_battle(env.console);
        BattleMenuWatcher battle_menu(BattleType::WILD);

        int ret = run_until(
            env, env.console,
            [=](const BotBaseContext& context){
                size_t stop = WALK_UP ? 30 : 60;
                for (size_t c = 0; c < stop; c++){
                    if (WALK_UP){
                        pbf_move_left_joystick(context, 128, 0, 125, 0);
                    }
                    pbf_mash_button(context, BUTTON_ZL, 125);
                }
            },
            {
                &start_battle,
                &battle_menu,
            }
        );
        switch (ret){
        case 0:
            env.log("Battle started!");
            break;
        case 1:
            env.log("Unexpected battle menu.", COLOR_RED);
            continue;
        default:
            env.log("Timed out.", COLOR_RED);
            continue;
        }

        //  Detect shiny.
        DoublesShinyDetection result = detect_shiny_battle(
            env.console,
            env, env.console, env.console,
            WILD_POKEMON,
            std::chrono::seconds(30)
        );

        bool stop = handler.handle_standard_encounter(result);
        if (stop){
            break;
        }
    }

    send_program_finished_notification(
        env.logger(), NOTIFICATION_PROGRAM_FINISH,
        env.program_info(),
        "",
        stats.to_str()
    );
    GO_HOME_WHEN_DONE.run_end_of_program(env.console);
}




}
}
}
