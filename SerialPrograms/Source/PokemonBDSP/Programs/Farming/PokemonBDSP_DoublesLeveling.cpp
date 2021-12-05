/*  Double Battle Leveling
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Inference/PokemonBDSP_StartBattleDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_EndBattleDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_ShinyEncounterDetector.h"
#include "PokemonBDSP/Programs/PokemonBDSP_EncounterHandler.h"
#include "PokemonBDSP_DoublesLeveling.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


DoublesLeveling_Descriptor::DoublesLeveling_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonBDSP:DoublesLeveling",
        STRING_POKEMON + " BDSP", "Double Battle Leveling",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/DoublesLeveling.md",
        "Level up your party by spamming spread moves in a double battle with a partner that heals you forever.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


DoublesLeveling::DoublesLeveling(const DoublesLeveling_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , ENCOUNTER_BOT_OPTIONS(false, false)
    , NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true)
    , NOTIFICATIONS({
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_NONSHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_SHINY,
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

    PA_ADD_OPTION(LANGUAGE);

    PA_ADD_OPTION(TRIGGER_METHOD);

    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
//    PA_ADD_OPTION(WATCHDOG_TIMER);
    PA_ADD_OPTION(EXIT_BATTLE_TIMEOUT);
}



struct DoublesLeveling::Stats : public PokemonSwSh::ShinyHuntTracker{
    Stats()
        : ShinyHuntTracker(false)
//        , m_resets(m_stats["Resets"])
    {
//        m_display_order.insert(m_display_order.begin() + 2, Stat("Resets"));
//        m_aliases["Unexpected Battles"] = "Errors";
    }
//    std::atomic<uint64_t>& m_resets;
};
std::unique_ptr<StatsTracker> DoublesLeveling::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



bool DoublesLeveling::find_encounter(SingleSwitchProgramEnvironment& env) const{
    BattleMenuDetector battle_menu_detector(BattleType::WILD);
    StartBattleDetector start_battle_detector(env.console);

    int result = run_until(
        env, env.console,
        [&](const BotBaseContext& context){
            while (true){
                TRIGGER_METHOD.run_trigger(context);
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
void DoublesLeveling::battle(SingleSwitchProgramEnvironment& env){
    Stats& stats = env.stats<Stats>();

    env.log("Starting battle!");

    //  State Machine
    for (size_t c = 0; c < 5; c++){
        env.console.botbase().wait_for_all_requests();

        BattleMenuDetector battle_menu(BattleType::WILD);
        EndBattleDetector end_battle;
        int ret = run_until(
            env, env.console,
            [=](const BotBaseContext& context){
                pbf_mash_button(context, BUTTON_B, 120 * TICKS_PER_SECOND);
            },
            {
                &battle_menu,
                &end_battle,
            }
        );
        switch (ret){
        case 0:
            env.log("Battle menu detected!", Qt::blue);
            pbf_mash_button(env.console, BUTTON_A, 5 * TICKS_PER_SECOND);
            break;
        case 1:
            env.log("Battle finished!", Qt::blue);
            pbf_mash_button(env.console, BUTTON_B, 250);
            return;
        default:
            env.log("Timed out.", Qt::red);
            stats.add_error();
            PA_THROW_StringException("Timed out after 2 minutes.");
        }
    }

    env.log("No progress detected after 5 battle menus.", Qt::red);
    PA_THROW_StringException("No progress detected after 5 battle menus. Are you out of PP?");
}



void DoublesLeveling::program(SingleSwitchProgramEnvironment& env){
    Stats& stats = env.stats<Stats>();
    env.update_stats();

    StandardEncounterHandler handler(
        env, env.console,
        LANGUAGE,
        ENCOUNTER_BOT_OPTIONS,
        stats
    );

    //  Connect the controller.
    pbf_press_button(env.console, BUTTON_B, 5, 5);

    //  Encounter Loop
    while (true){
        //  Find encounter.
        bool battle = find_encounter(env);
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

        bool stop = handler.handle_standard_encounter(result);
        if (stop){
            break;
        }

        this->battle(env);
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
