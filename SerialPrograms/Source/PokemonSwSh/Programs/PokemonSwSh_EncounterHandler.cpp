/*  Encounter Handler
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "Common/Cpp/Exception.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/VisualInferenceSession.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSwSh/Programs/PokemonSwSh_BasicCatcher.h"
#include "PokemonSwSh_EncounterHandler.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void take_video(const BotBaseContext& context){
    pbf_wait(context, 5 * TICKS_PER_SECOND);
    pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
}
void run_away(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    uint16_t exit_battle_time
){
    InterruptableCommandSession commands(console);

    BlackScreenDetector black_screen_detector(console);
    black_screen_detector.register_command_stop(commands);

    AsyncVisualInferenceSession inference(env, console);
    inference += black_screen_detector;

    commands.run([=](const BotBaseContext& context){
        pbf_press_dpad(context, DPAD_UP, 10, 0);
        pbf_mash_button(context, BUTTON_A, TICKS_PER_SECOND);
        if (exit_battle_time > TICKS_PER_SECOND){
            pbf_mash_button(context, BUTTON_B, exit_battle_time - TICKS_PER_SECOND);
        }
        context.botbase().wait_for_all_requests();
    });
}




StandardEncounterHandler::StandardEncounterHandler(
    const QString& program_name,
    ProgramEnvironment& env,
    ConsoleHandle& console,
    Language language,
    const EncounterBotCommonSettings& settings,
    ShinyHuntTracker& session_stats
)
    : m_program_name(program_name)
    , m_env(env)
    , m_console(console)
    , m_language(language)
    , m_settings(settings)
    , m_session_stats(session_stats)
    , m_notification_sender(settings.notification_level)
{}


void StandardEncounterHandler::update_frequencies(StandardEncounterDetection& encounter){
    const std::set<std::string>* slugs = encounter.candidates();
    if (slugs){
        m_frequencies += *slugs;
        m_env.log(m_frequencies.dump_sorted_map("Encounter Stats:\n"));
    }
}
void StandardEncounterHandler::run_away_and_update_stats(
    StandardEncounterDetection& encounter,
    uint16_t exit_battle_time,
    const ShinyDetectionResult& result
){
    //  Initiate the run-away.
    pbf_press_dpad(m_console, DPAD_UP, 10, 0);
    pbf_mash_button(m_console, BUTTON_A, 60);
    m_console.botbase().wait_for_all_requests();

    update_frequencies(encounter);

    m_notification_sender.send_notification(
        m_console,
        m_program_name,
        encounter.candidates(),
        result, m_settings.shiny_screenshot,
        &m_session_stats,
        &m_frequencies
    );

    InterruptableCommandSession commands(m_console);

    BlackScreenDetector black_screen_detector(m_console);
    black_screen_detector.register_command_stop(commands);

    AsyncVisualInferenceSession inference(m_env, m_console);
    inference += black_screen_detector;

    commands.run([=](const BotBaseContext& context){
        pbf_mash_button(context, BUTTON_A, TICKS_PER_SECOND);
        if (exit_battle_time > TICKS_PER_SECOND){
            pbf_mash_button(context, BUTTON_B, exit_battle_time - TICKS_PER_SECOND);
        }
        context.botbase().wait_for_all_requests();
    });

}


bool StandardEncounterHandler::handle_standard_encounter(const ShinyDetectionResult& result){
    m_session_stats += result.shiny_type;
    m_env.update_stats();

    if (result.shiny_type == ShinyType::UNKNOWN){
        pbf_mash_button(m_console, BUTTON_B, TICKS_PER_SECOND);
        return false;
    }

    StandardEncounterDetection encounter(
        m_env, m_console,
        m_language,
        m_settings.filter,
        result.shiny_type
    );

    update_frequencies(encounter);

    m_notification_sender.send_notification(
        m_console,
        m_program_name,
        encounter.candidates(),
        result, m_settings.shiny_screenshot,
        &m_session_stats,
        &m_frequencies
    );

    if (m_settings.video_on_shiny && encounter.is_shiny()){
        take_video(m_console);
    }

    return encounter.get_action().first == EncounterAction::StopProgram;
}
bool StandardEncounterHandler::handle_standard_encounter_end_battle(
    const ShinyDetectionResult& result,
    uint16_t exit_battle_time
){
    m_session_stats += result.shiny_type;
    m_env.update_stats();

    if (result.shiny_type == ShinyType::UNKNOWN){
        pbf_mash_button(m_console, BUTTON_B, TICKS_PER_SECOND);
        run_away(m_env, m_console, exit_battle_time);
        return false;
    }

    StandardEncounterDetection encounter(
        m_env, m_console,
        m_language,
        m_settings.filter,
        result.shiny_type
    );

    if (m_settings.video_on_shiny && encounter.is_shiny()){
        take_video(m_console);
    }

    std::pair<EncounterAction, std::string> action  = encounter.get_action();

    //  Fast run-away sequence to save time.
    if (action.first == EncounterAction::RunAway){
        run_away_and_update_stats(encounter, exit_battle_time, result);
        return false;
    }

    update_frequencies(encounter);
    m_notification_sender.send_notification(
        m_console,
        m_program_name,
        encounter.candidates(),
        result, m_settings.shiny_screenshot,
        &m_session_stats,
        &m_frequencies
    );

    switch (action.first){
    case EncounterAction::StopProgram:
        return true;
    case EncounterAction::RunAway:
        return false;
    case EncounterAction::ThrowBalls:{
        CatchResults results = basic_catcher(m_env, m_console, m_language, action.second);
        switch (results.result){
        case CatchResult::POKEMON_CAUGHT:
        case CatchResult::POKEMON_FAINTED:
            break;
        case CatchResult::OWN_FAINTED:
            PA_THROW_StringException("Your " + STRING_POKEMON + " fainted after " + QString::number(results.balls_used) + " balls.");
        case CatchResult::OUT_OF_BALLS:
            PA_THROW_StringException("Unable to find the desired ball after throwing " + QString::number(results.balls_used) + " of them. Did you run out?");
        case CatchResult::TIMEOUT:
            PA_THROW_StringException("Program has timed out. Did your lead " + STRING_POKEMON + " faint?");
        }
        send_catch_notification(
            m_console,
            m_program_name,
            encounter.candidates(),
            action.second,
            results.balls_used,
            results.result == CatchResult::POKEMON_CAUGHT,
            result.shiny_type != ShinyType::NOT_SHINY
        );
        return false;
    }
    case EncounterAction::ThrowBallsAndSave:{
        CatchResults results = basic_catcher(m_env, m_console, m_language, action.second);
        switch (results.result){
        case CatchResult::POKEMON_CAUGHT:
            pbf_wait(m_console, 2 * TICKS_PER_SECOND);
            pbf_press_button(m_console, BUTTON_X, 20, OVERWORLD_TO_MENU_DELAY); //  Save game.
            pbf_press_button(m_console, BUTTON_R, 20, 150);
            pbf_press_button(m_console, BUTTON_A, 10, 500);
            break;
        case CatchResult::POKEMON_FAINTED:
            break;
        case CatchResult::OWN_FAINTED:
            PA_THROW_StringException("Your " + STRING_POKEMON + " fainted after " + QString::number(results.balls_used) + " balls.");
        case CatchResult::OUT_OF_BALLS:
            PA_THROW_StringException("Unable to find the desired ball after throwing " + QString::number(results.balls_used) + " of them. Did you run out?");
        case CatchResult::TIMEOUT:
            PA_THROW_StringException("Program has timed out. Did your lead " + STRING_POKEMON + " faint?");
        }
        send_catch_notification(
            m_console,
            m_program_name,
            encounter.candidates(),
            action.second,
            results.balls_used,
            results.result == CatchResult::POKEMON_CAUGHT,
            result.shiny_type != ShinyType::NOT_SHINY
        );
        return false;
    }
    }

    return false;
}



}
}
}
