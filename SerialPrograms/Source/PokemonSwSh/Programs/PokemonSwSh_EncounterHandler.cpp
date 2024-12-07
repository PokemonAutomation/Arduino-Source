/*  Encounter Handler
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Programs/PokemonSwSh_BasicCatcher.h"
#include "PokemonSwSh_EncounterHandler.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void take_video(BotBaseContext& context){
    pbf_wait(context, 5 * TICKS_PER_SECOND);
    pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
//    context->wait_for_all_requests();
}
void run_away(
    ConsoleHandle& console, BotBaseContext& context,
    uint16_t exit_battle_time
){
    BlackScreenOverWatcher black_screen_detector;
    run_until(
        console, context,
        [exit_battle_time](BotBaseContext& context){
            pbf_press_dpad(context, DPAD_UP, 10, 0);
            pbf_mash_button(context, BUTTON_A, TICKS_PER_SECOND);
            if (exit_battle_time > TICKS_PER_SECOND){
                pbf_mash_button(context, BUTTON_B, exit_battle_time - TICKS_PER_SECOND);
            }
        },
        {{black_screen_detector}}
    );
}




StandardEncounterHandler::StandardEncounterHandler(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    Language language,
    EncounterBotCommonOptions& settings,
    ShinyHuntTracker& session_stats
)
    : m_env(env)
    , m_context(context)
    , m_console(console)
    , m_language(language)
    , m_settings(settings)
    , m_session_stats(session_stats)
//    , m_notification_sender(settings.notification_level)
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
    pbf_press_dpad(m_context, DPAD_UP, 10, 0);
    pbf_mash_button(m_context, BUTTON_A, 60);
    m_context.wait_for_all_requests();

    update_frequencies(encounter);

    const std::set<std::string>* candidates_ptr = encounter.candidates();
    const std::set<std::string>& candidates = candidates_ptr
        ? *candidates_ptr
        : std::set<std::string>();
    send_encounter_notification(
        m_env,
        m_settings.NOTIFICATION_NONSHINY,
        m_settings.NOTIFICATION_SHINY,
        candidates_ptr, is_likely_shiny(result.shiny_type),
        {{candidates, result.shiny_type}}, result.alpha,
        result.get_best_screenshot(),
        &m_frequencies
    );

    BlackScreenOverWatcher black_screen_detector;
    int ret = run_until(
        m_console, m_context,
        [exit_battle_time](BotBaseContext& context){
            pbf_mash_button(context, BUTTON_A, TICKS_PER_SECOND);
            if (exit_battle_time > TICKS_PER_SECOND){
                pbf_mash_button(context, BUTTON_B, exit_battle_time - TICKS_PER_SECOND);
            }
        },
        {{black_screen_detector}}
    );
    if (ret < 0){
        m_console.log("Timed out waiting for end of battle. Are you stuck in the battle?", COLOR_RED);
    }
}


bool StandardEncounterHandler::handle_standard_encounter(const ShinyDetectionResult& result){
    if (result.shiny_type == ShinyType::UNKNOWN){
        m_console.log("Unable to determine result of battle.", COLOR_RED);
        m_session_stats.add_error();
        m_consecutive_failures++;
        if (m_consecutive_failures >= 3){
            OperationFailedException::fire(
                m_console, ErrorReport::SEND_ERROR_REPORT,
                "3 consecutive failed encounter detections."
            );
        }
        return false;
    }
    m_consecutive_failures = 0;

    m_session_stats += result.shiny_type;
    m_env.update_stats();

    if (result.shiny_type == ShinyType::UNKNOWN){
        pbf_mash_button(m_context, BUTTON_B, TICKS_PER_SECOND);
        return false;
    }

    StandardEncounterDetection encounter(
        m_env, m_console, m_context,
        m_language,
        m_settings.FILTER,
        result.shiny_type
    );

    update_frequencies(encounter);

    const std::set<std::string>* candidates_ptr = encounter.candidates();
    const std::set<std::string>& candidates = candidates_ptr
        ? *candidates_ptr
        : std::set<std::string>();
    send_encounter_notification(
        m_env,
        m_settings.NOTIFICATION_NONSHINY,
        m_settings.NOTIFICATION_SHINY,
        candidates_ptr, is_likely_shiny(result.shiny_type),
        {{candidates, result.shiny_type}}, result.alpha,
        result.get_best_screenshot(),
        &m_frequencies
    );

    if (m_settings.VIDEO_ON_SHINY && encounter.is_shiny()){
        take_video(m_context);
    }

    return encounter.get_action().action == EncounterAction::StopProgram;
}
bool StandardEncounterHandler::handle_standard_encounter_end_battle(
    const ShinyDetectionResult& result,
    uint16_t exit_battle_time
){
    if (result.shiny_type == ShinyType::UNKNOWN){
        m_console.log("Unable to determine result of battle.", COLOR_RED);
        m_session_stats.add_error();
        m_consecutive_failures++;
        if (m_consecutive_failures >= 3){
            OperationFailedException::fire(
                m_console, ErrorReport::SEND_ERROR_REPORT,
                "3 consecutive failed encounter detections."
            );
        }
        return false;
    }
    m_consecutive_failures = 0;

    m_session_stats += result.shiny_type;
    m_env.update_stats();

    StandardEncounterDetection encounter(
        m_env, m_console, m_context,
        m_language,
        m_settings.FILTER,
        result.shiny_type
    );

    if (m_settings.VIDEO_ON_SHINY && encounter.is_shiny()){
        take_video(m_context);
    }

    EncounterActionFull action = encounter.get_action();

//    cout << "action = " << (int)action.first << " : " << action.second << endl;

    //  Fast run-away sequence to save time.
    if (action.action == EncounterAction::RunAway){
        run_away_and_update_stats(encounter, exit_battle_time, result);
        return false;
    }

    update_frequencies(encounter);
    const std::set<std::string>* candidates_ptr = encounter.candidates();
    const std::set<std::string>& candidates = candidates_ptr
        ? *candidates_ptr
        : std::set<std::string>();
    send_encounter_notification(
        m_env,
        m_settings.NOTIFICATION_NONSHINY,
        m_settings.NOTIFICATION_SHINY,
        candidates_ptr, is_likely_shiny(result.shiny_type),
        {{candidates, result.shiny_type}}, result.alpha,
        result.get_best_screenshot(),
        &m_frequencies
    );

    switch (action.action){
    case EncounterAction::StopProgram:
        return true;
    case EncounterAction::RunAway:
        return false;
    case EncounterAction::ThrowBalls:{
        CatchResults results = basic_catcher(m_console, m_context, m_language, action.pokeball_slug, action.ball_limit);
        send_catch_notification(
            m_env,
            m_settings.NOTIFICATION_CATCH_SUCCESS,
            m_settings.NOTIFICATION_CATCH_FAILED,
            encounter.candidates(),
            action.pokeball_slug,
            results.balls_used,
            results.result
        );
        switch (results.result){
        case CatchResult::POKEMON_CAUGHT:
        case CatchResult::POKEMON_FAINTED:
            break;
        default:
            throw_and_log<FatalProgramException>(
                m_console, ErrorReport::NO_ERROR_REPORT,
                "Unable to recover from failed catch.",
                m_console
            );
        }
        return false;
    }
    case EncounterAction::ThrowBallsAndSave:{
        CatchResults results = basic_catcher(m_console, m_context, m_language, action.pokeball_slug, action.ball_limit);
        send_catch_notification(
            m_env,
            m_settings.NOTIFICATION_CATCH_SUCCESS,
            m_settings.NOTIFICATION_CATCH_FAILED,
            encounter.candidates(),
            action.pokeball_slug,
            results.balls_used,
            results.result
        );
        switch (results.result){
        case CatchResult::POKEMON_CAUGHT:
            pbf_mash_button(m_context, BUTTON_B, 2 * TICKS_PER_SECOND);
            pbf_press_button(m_context, BUTTON_X, 20, GameSettings::instance().OVERWORLD_TO_MENU_DELAY); //  Save game.
            pbf_press_button(m_context, BUTTON_R, 20, 150);
            pbf_press_button(m_context, BUTTON_A, 10, 500);
            break;
        case CatchResult::POKEMON_FAINTED:
            pbf_mash_button(m_context, BUTTON_B, 2 * TICKS_PER_SECOND);
            break;
        default:
            throw_and_log<FatalProgramException>(
                m_console, ErrorReport::NO_ERROR_REPORT,
                "Unable to recover from failed catch.",
                m_console
            );
        }
        return false;
    }
    }

    return false;
}



}
}
}
