/*  Encounter Handler
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QtGlobal>
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP_BasicCatcher.h"
#include "PokemonBDSP_EncounterHandler.h"
#include "PokemonBDSP_GameNavigation.h"
#include "PokemonBDSP_RunFromBattle.h"

#include <iostream>
#include <sstream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



void take_video(ProControllerContext& context){
    pbf_wait(context, 5 * TICKS_PER_SECOND);
    pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
//    context->wait_for_all_requests();
}


StandardEncounterHandler::StandardEncounterHandler(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
    Language language,
    EncounterBotCommonOptions& settings,
    PokemonSwSh::ShinyHuntTracker& session_stats
)
    : m_env(env)
    , m_context(context)
    , m_stream(stream)
    , m_language(language)
    , m_settings(settings)
    , m_session_stats(session_stats)
{}



std::vector<PokemonDetection> get_mon_list(StandardEncounterDetection& encounter){
    std::vector<PokemonDetection> mon_list;
    const PokemonDetection& left = encounter.pokemon_left();
    const PokemonDetection& right = encounter.pokemon_right();
    if (left.exists){
        mon_list.emplace_back(left);
    }
    if (right.exists){
        mon_list.emplace_back(right);
    }
    return mon_list;
}

void StandardEncounterHandler::run_away_due_to_error(Milliseconds exit_battle_time){
    pbf_mash_button(m_context, BUTTON_B, 3000ms);
    pbf_press_dpad(m_context, DPAD_DOWN, 3000ms, 0ms);
    m_context.wait_for_all_requests();

    run_from_battle(m_stream, m_context, exit_battle_time);
}

std::vector<EncounterResult> StandardEncounterHandler::results(StandardEncounterDetection& encounter){
    std::vector<EncounterResult> ret;
    const PokemonDetection& left = encounter.pokemon_left();
    const PokemonDetection& right = encounter.pokemon_right();
    if (left.exists){
        ret.emplace_back(EncounterResult{left.slugs, encounter.left_shininess()});
    }
    if (right.exists){
        ret.emplace_back(EncounterResult{right.slugs, encounter.right_shininess()});
    }
    return ret;
}
void StandardEncounterHandler::update_frequencies(StandardEncounterDetection& encounter){
    const PokemonDetection& left = encounter.pokemon_left();
    const PokemonDetection& right = encounter.pokemon_right();
    if (!left.detection_enabled && !right.detection_enabled){
        return;
    }
    if (left.exists){
        m_frequencies += left.slugs;
    }
    if (right.exists){
        m_frequencies += right.slugs;
    }
    if (left.exists || right.exists){
        m_env.log(m_frequencies.dump_sorted_map("Encounter Stats:\n"));
    }
}


bool StandardEncounterHandler::handle_standard_encounter(const DoublesShinyDetection& result){
    if (result.shiny_type == ShinyType::UNKNOWN){
        m_stream.log("Unable to determine result of battle.", COLOR_RED);
        m_session_stats.add_error();
        m_consecutive_failures++;
        if (m_consecutive_failures >= 3){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "3 consecutive failed encounter detections.",
                m_stream
            );
        }
        return false;
    }
    m_consecutive_failures = 0;

    StandardEncounterDetection encounter(
        m_stream, m_context,
        m_language,
        m_settings.FILTER,
        result
    );

    m_session_stats += result.shiny_type;
    if (encounter.is_double_battle()){
        bool left = is_confirmed_shiny(encounter.left_shininess());
        bool right = is_confirmed_shiny(encounter.right_shininess());
        if (left && right){
            m_session_stats += ShinyType::UNKNOWN_SHINY;
        }else{
            m_session_stats += ShinyType::NOT_SHINY;
        }
    }
    m_env.update_stats();

    if (result.shiny_type == ShinyType::UNKNOWN){
        pbf_mash_button(m_context, BUTTON_B, TICKS_PER_SECOND);
        return false;
    }

    bool enable_names = m_language != Language::None;
    std::vector<EncounterResult> encounter_results = results(encounter);

    update_frequencies(encounter);
    send_encounter_notification(
        m_env,
        m_settings.NOTIFICATION_NONSHINY,
        m_settings.NOTIFICATION_SHINY,
        m_language != Language::None, is_likely_shiny(result.shiny_type),
        encounter_results, result.alpha,
        result.get_best_screenshot(),
        enable_names ? &m_frequencies : nullptr
    );

    if (m_settings.VIDEO_ON_SHINY && encounter.has_shiny()){
        take_video(m_context);
    }

    return encounter.get_action().action == EncounterAction::StopProgram;
}
bool StandardEncounterHandler::handle_standard_encounter_end_battle(
    const DoublesShinyDetection& result,
    Milliseconds exit_battle_time
){
    if (result.shiny_type == ShinyType::UNKNOWN){
        m_stream.log("Unable to determine result of battle.", COLOR_RED);
        m_session_stats.add_error();
        m_consecutive_failures++;
        if (m_consecutive_failures >= 3){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "3 consecutive failed encounter detections.",
                m_stream
            );
        }
        return false;
    }
    m_consecutive_failures = 0;

    StandardEncounterDetection encounter(
        m_stream, m_context,
        m_language,
        m_settings.FILTER,
        result
    );

    m_session_stats += result.shiny_type;
    if (encounter.is_double_battle()){
        bool left = is_confirmed_shiny(encounter.left_shininess());
        bool right = is_confirmed_shiny(encounter.right_shininess());
        if (left && right){
            m_session_stats += ShinyType::UNKNOWN_SHINY;
        }else{
            m_session_stats += ShinyType::NOT_SHINY;
        }
    }
    m_env.update_stats();

    if (m_settings.VIDEO_ON_SHINY && encounter.has_shiny()){
        take_video(m_context);
    }

    bool enable_names = m_language != Language::None;
    std::vector<EncounterResult> encounter_results = results(encounter);
    std::ostringstream os;
    if (encounter_results.size() > 0){
        os << "Pokemon: (";
        bool first_slug = true;
        for(const auto& name: encounter_results[0].slug_candidates){
            if (first_slug == false){
                os << ",";
            }
            os << name;
            first_slug = false;
        }
        os << ")";
    }
    if (encounter_results.size() > 1){
        os << "(";
        bool first_slug = true;
        for(const auto& name: encounter_results[1].slug_candidates){
            if (first_slug == false){
                os << ",";
            }
            os << name;
            first_slug = false;
        }
        os << ")";
    }
    m_stream.overlay().add_log(os.str(), COLOR_WHITE);

    update_frequencies(encounter);
    send_encounter_notification(
        m_env,
        m_settings.NOTIFICATION_NONSHINY,
        m_settings.NOTIFICATION_SHINY,
        enable_names, is_likely_shiny(result.shiny_type),
        encounter_results, result.alpha,
        result.get_best_screenshot(),
        enable_names ? &m_frequencies : nullptr
    );

    EncounterActionFull action = encounter.get_action();
    switch (action.action){
    case EncounterAction::StopProgram:
        return true;
    case EncounterAction::RunAway:
        //  Fast run-away sequence to save time.
        pbf_press_dpad(m_context, DPAD_UP, 20, 0);
        m_context.wait_for_all_requests();

        run_from_battle(m_stream, m_context, exit_battle_time);
        return false;

    case EncounterAction::ThrowBalls:
    case EncounterAction::ThrowBallsAndSave:{
        CatchResults catch_result = basic_catcher(
            m_stream, m_context,
            m_language,
            action.pokeball_slug,
            action.ball_limit
        );
        send_catch_notification(
            m_env,
            m_settings.NOTIFICATION_CATCH_SUCCESS,
            m_settings.NOTIFICATION_CATCH_FAILED,
            &encounter_results[0].slug_candidates,
            action.pokeball_slug,
            catch_result.balls_used,
            catch_result.result
        );
        switch (catch_result.result){
        case CatchResult::POKEMON_CAUGHT:
            m_session_stats.add_caught();
            m_env.update_stats();
            if (action.action == EncounterAction::ThrowBallsAndSave){
                //  Save the game
                save_game(m_stream, m_context);
            }
            break;
        case CatchResult::POKEMON_FAINTED:
            pbf_mash_button(m_context, BUTTON_B, 2 * TICKS_PER_SECOND);
            break;
        default:
            throw_and_log<FatalProgramException>(
                m_stream.logger(), ErrorReport::NO_ERROR_REPORT,
                "Unable to recover from failed catch.",
                m_stream
            );
        }
        return false;
    }
    default:
        return true;
    }

    return false;
}





LeadingShinyTracker::LeadingShinyTracker(Logger& logger)
    : m_logger(logger)
    , m_consecutive_shinies(0)
{}

void LeadingShinyTracker::report_result(ShinyType type){
    if (is_confirmed_shiny(type)){
        m_consecutive_shinies++;
//        cout << "own shiny = " << m_consecutive_shinies << endl;
        if (m_consecutive_shinies >= 3){
            throw UserSetupError(m_logger, "Don't use a shiny as your lead. It causes false positive detections.");
        }
    }else{
        m_consecutive_shinies = 0;
    }
}








}
}
}
