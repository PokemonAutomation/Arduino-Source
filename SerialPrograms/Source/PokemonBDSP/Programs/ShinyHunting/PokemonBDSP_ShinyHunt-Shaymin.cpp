/*  Shiny Hunt - Shaymin Runaway
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
#include "PokemonBDSP/Inference/PokemonBDSP_StartBattleDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_ShinyEncounterDetector.h"
#include "PokemonBDSP/Programs/PokemonBDSP_EncounterHandler.h"
#include "PokemonBDSP_ShinyHunt-Shaymin.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


ShinyHuntShaymin_Descriptor::ShinyHuntShaymin_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonBDSP:ShinyHuntShaymin",
        STRING_POKEMON + " BDSP", "Shiny Hunt - Shaymin",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/ShinyHuntShaymin-Overworld.md",
        "Shiny hunt Shaymin using the runaway method.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


ShinyHuntShaymin::ShinyHuntShaymin(const ShinyHuntShaymin_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , SHORTCUT("<b>Bike Shortcut:</b>")
    , ENCOUNTER_BOT_OPTIONS(false, false)
    , NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true)
    , NOTIFICATIONS({
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_NONSHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_SHINY,
//        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_SUCCESS,
//        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_FAILED,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_PROGRAM_ERROR,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , EXIT_BATTLE_TIMEOUT(
        "<b>Exit Battle Timeout:</b><br>After running, wait this long to return to overworld.",
        "10 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(SHORTCUT);

    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
//    PA_ADD_OPTION(WATCHDOG_TIMER);
    PA_ADD_OPTION(EXIT_BATTLE_TIMEOUT);
}



std::unique_ptr<StatsTracker> ShinyHuntShaymin::make_stats() const{
    return std::unique_ptr<StatsTracker>(new PokemonSwSh::ShinyHuntTracker(false));
}



bool ShinyHuntShaymin::start_encounter(SingleSwitchProgramEnvironment& env) const{
    BattleMenuDetector battle_menu_detector(BattleType::WILD);
    StartBattleDetector start_battle_detector(env.console);

    int result = run_until(
        env, env.console,
        [&](const BotBaseContext& context){
            while (true){
                pbf_mash_button(context, BUTTON_ZL, 125);
            }
        },
        {
            &battle_menu_detector,
            &start_battle_detector,
        }
    );

    switch (result){
    case 0:
        env.console.log("Unexpected Battle.", "red");
        return false;
    case 1:
        env.console.log("Battle started!");
        return true;
    }
    return false;
}

void ShinyHuntShaymin::program(SingleSwitchProgramEnvironment& env){
    PokemonSwSh::ShinyHuntTracker& stats = env.stats<PokemonSwSh::ShinyHuntTracker>();
    env.update_stats();

    StandardEncounterHandler handler(
        env, env.console,
        Language::None,
        ENCOUNTER_BOT_OPTIONS,
        stats
    );

    //  Connect the controller.
    pbf_press_button(env.console, BUTTON_B, 5, 5);

    //  Encounter Loop
    while (true){
        //  Find encounter.
        bool battle = start_encounter(env);
        if (!battle){
            stats.add_error();
            handler.run_away_due_to_error(EXIT_BATTLE_TIMEOUT);
            continue;
        }

        //  Detect shiny.
        DoublesShinyDetection result = detect_shiny_battle(
            env.console,
            env, env.console, env.console,
            WILD_POKEMON,
            std::chrono::seconds(30)
        );

        bool stop = handler.handle_standard_encounter_end_battle(result, EXIT_BATTLE_TIMEOUT);
        if (stop){
            break;
        }

        //  Clear dialogs.
        pbf_mash_button(env.console, BUTTON_B, 75);

        //  Hop on bike, ride down to seabreak path
        SHORTCUT.run(env.console, 0);
        pbf_move_left_joystick(env.console, 128, 255, 315, 0);
        pbf_move_left_joystick(env.console, 128, 0, 315, 0);
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
