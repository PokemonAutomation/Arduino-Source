/*  Encounter Handler
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "Pokemon/Pokemon_Types.h"
#include "PokemonBDSP_EncounterHandler.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



void take_video(const BotBaseContext& context){
    pbf_wait(context, 5 * TICKS_PER_SECOND);
    pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
//    context->wait_for_all_requests();
}
void run_away(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    uint16_t exit_battle_time
){
    BlackScreenOverDetector black_screen_detector;
    run_until(
        env, console,
        [=](const BotBaseContext& context){
            pbf_press_dpad(context, DPAD_UP, 10, 0);
            pbf_mash_button(context, BUTTON_A, TICKS_PER_SECOND);
            if (exit_battle_time > TICKS_PER_SECOND){
                pbf_mash_button(context, BUTTON_B, exit_battle_time - TICKS_PER_SECOND);
            }
        },
        { &black_screen_detector }
    );
}


StandardEncounterHandler::StandardEncounterHandler(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    Language language,
    EncounterBotCommonOptions& settings,
    PokemonSwSh::ShinyHuntTracker& session_stats
)
    : m_env(env)
    , m_console(console)
    , m_language(language)
    , m_settings(settings)
    , m_session_stats(session_stats)
{}



std::vector<std::set<std::string>> get_mon_list(StandardEncounterDetection& encounter){
    std::vector<std::set<std::string>> mon_list;
    const std::set<std::string>* left = encounter.pokemon_left();
    const std::set<std::string>* right = encounter.pokemon_right();
    if (left){
        mon_list.emplace_back(*left);
    }
    if (right){
        mon_list.emplace_back(*right);
    }
    return mon_list;
}

void StandardEncounterHandler::run_away(uint16_t exit_battle_time){
    pbf_mash_button(m_console, BUTTON_B, 3 * TICKS_PER_SECOND);
    pbf_press_dpad(m_console, DPAD_DOWN, 3 * TICKS_PER_SECOND, 0);
    m_console.botbase().wait_for_all_requests();

    BlackScreenOverDetector black_screen_detector;
    int ret = run_until(
        m_env, m_console,
        [=](const BotBaseContext& context){
            pbf_mash_button(context, BUTTON_A, TICKS_PER_SECOND);
            if (exit_battle_time > TICKS_PER_SECOND){
                pbf_mash_button(context, BUTTON_B, exit_battle_time - TICKS_PER_SECOND);
            }
        },
        { &black_screen_detector }
    );
    if (ret < 0){
        m_console.log("Timed out waiting for end of battle. Are you stuck in the battle?", Qt::red);
        m_session_stats.add_error();
    }
    pbf_wait(m_console, TICKS_PER_SECOND);
    m_console.botbase().wait_for_all_requests();
}

std::vector<EncounterResult> StandardEncounterHandler::results(StandardEncounterDetection& encounter){
    std::vector<EncounterResult> ret;
    const std::set<std::string>* slugs0 = encounter.pokemon_left();
    const std::set<std::string>* slugs1 = encounter.pokemon_right();
    if (slugs0){
        ret.emplace_back(EncounterResult{*slugs0, encounter.left_shininess()});
    }
    if (slugs1){
        ret.emplace_back(EncounterResult{*slugs1, encounter.right_shininess()});
    }
    return ret;
}
void StandardEncounterHandler::update_frequencies(StandardEncounterDetection& encounter){
    const std::set<std::string>* slugs0 = encounter.pokemon_left();
    const std::set<std::string>* slugs1 = encounter.pokemon_right();
    if (slugs0){
        m_frequencies += *slugs0;
    }
    if (slugs1){
        m_frequencies += *slugs1;
    }
    if (slugs0 || slugs1){
        m_env.log(m_frequencies.dump_sorted_map("Encounter Stats:\n"));
    }
}
void StandardEncounterHandler::run_away_and_update_stats(
    StandardEncounterDetection& encounter,
    uint16_t exit_battle_time,
    const DoublesShinyDetection& result
){
    pbf_press_dpad(m_console, DPAD_UP, 10, 0);

    update_frequencies(encounter);
    send_encounter_notification(
        m_console,
        m_settings.NOTIFICATION_NONSHINY,
        m_settings.NOTIFICATION_SHINY,
        m_env.program_info(),
        m_language != Language::None, is_shiny(result.shiny_type),
        results(encounter),
        result.best_screenshot,
        &m_session_stats,
        &m_frequencies
    );

    m_console.botbase().wait_for_all_requests();

    BlackScreenOverDetector black_screen_detector;
    int ret = run_until(
        m_env, m_console,
        [=](const BotBaseContext& context){
            pbf_mash_button(context, BUTTON_A, TICKS_PER_SECOND);
            if (exit_battle_time > TICKS_PER_SECOND){
                pbf_mash_button(context, BUTTON_B, exit_battle_time - TICKS_PER_SECOND);
            }
        },
        { &black_screen_detector }
    );
    if (ret < 0){
        m_console.log("Timed out waiting for end of battle. Are you stuck in the battle?", Qt::red);
        m_session_stats.add_error();
    }
    pbf_wait(m_console, TICKS_PER_SECOND);
    m_console.botbase().wait_for_all_requests();
}


bool StandardEncounterHandler::handle_standard_encounter(const DoublesShinyDetection& result){
    if (result.shiny_type == ShinyType::UNKNOWN){
        m_console.log("Unable to determine result of battle.", Qt::red);
        m_session_stats.add_error();
        return false;
    }

    StandardEncounterDetection encounter(
        m_env, m_console,
        m_language,
        m_settings.FILTER,
        result
    );

    m_session_stats += result.shiny_type;
    if (encounter.is_double_battle()){
        bool left = is_shiny(encounter.left_shininess());
        bool right = is_shiny(encounter.right_shininess());
        if (left && right){
            m_session_stats += ShinyType::UNKNOWN_SHINY;
        }else{
            m_session_stats += ShinyType::NOT_SHINY;
        }
    }
    m_env.update_stats();

    if (result.shiny_type == ShinyType::UNKNOWN){
        pbf_mash_button(m_console, BUTTON_B, TICKS_PER_SECOND);
        return false;
    }

    update_frequencies(encounter);
    send_encounter_notification(
        m_console,
        m_settings.NOTIFICATION_NONSHINY,
        m_settings.NOTIFICATION_SHINY,
        m_env.program_info(),
        m_language != Language::None, is_shiny(result.shiny_type),
        results(encounter),
        result.best_screenshot,
        &m_session_stats,
        &m_frequencies
    );

    if (m_settings.VIDEO_ON_SHINY && encounter.has_shiny()){
        take_video(m_console);
    }

    return encounter.get_action().first == EncounterAction::StopProgram;
}
bool StandardEncounterHandler::handle_standard_encounter_end_battle(
    const DoublesShinyDetection& result,
    uint16_t exit_battle_time
){
    if (result.shiny_type == ShinyType::UNKNOWN){
        m_console.log("Unable to determine result of battle.", Qt::red);
        m_session_stats.add_error();
        return false;
    }

    StandardEncounterDetection encounter(
        m_env, m_console,
        m_language,
        m_settings.FILTER,
        result
    );

    m_session_stats += result.shiny_type;
    if (encounter.is_double_battle()){
        bool left = is_shiny(encounter.left_shininess());
        bool right = is_shiny(encounter.right_shininess());
        if (left && right){
            m_session_stats += ShinyType::UNKNOWN_SHINY;
        }else{
            m_session_stats += ShinyType::NOT_SHINY;
        }
    }
    m_env.update_stats();

    if (m_settings.VIDEO_ON_SHINY && encounter.has_shiny()){
        take_video(m_console);
    }

    std::pair<EncounterAction, std::string> action = encounter.get_action();

    //  Fast run-away sequence to save time.
    if (action.first == EncounterAction::RunAway){
        run_away_and_update_stats(encounter, exit_battle_time, result);
        return false;
    }

    update_frequencies(encounter);
    send_encounter_notification(
        m_console,
        m_settings.NOTIFICATION_NONSHINY,
        m_settings.NOTIFICATION_SHINY,
        m_env.program_info(),
        m_language != Language::None, is_shiny(result.shiny_type),
        results(encounter),
        result.best_screenshot,
        &m_session_stats,
        &m_frequencies
    );

    switch (action.first){
    case EncounterAction::StopProgram:
        return true;
    case EncounterAction::RunAway:
        return false;
    default:
        return true;
    }

    return false;
}




}
}
}
