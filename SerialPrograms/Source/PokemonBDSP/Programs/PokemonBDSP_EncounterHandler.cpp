/*  Encounter Handler
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Types.h"
#include "PokemonBDSP_RunFromBattle.h"
#include "PokemonBDSP_EncounterHandler.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



void take_video(const BotBaseContext& context){
    pbf_wait(context, 5 * TICKS_PER_SECOND);
    pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
//    context->wait_for_all_requests();
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

void StandardEncounterHandler::run_away_due_to_error(uint16_t exit_battle_time){
    pbf_mash_button(m_console, BUTTON_B, 3 * TICKS_PER_SECOND);
    pbf_press_dpad(m_console, DPAD_DOWN, 3 * TICKS_PER_SECOND, 0);
    m_console.botbase().wait_for_all_requests();

    run_from_battle(m_env, m_console, exit_battle_time);
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
void StandardEncounterHandler::run_away_and_update_stats(
    StandardEncounterDetection& encounter,
    uint16_t exit_battle_time,
    const DoublesShinyDetection& result
){
    pbf_press_dpad(m_console, DPAD_UP, 10, 0);

    bool enable_names = m_language != Language::None;
    update_frequencies(encounter);
    send_encounter_notification(
        m_console,
        m_settings.NOTIFICATION_NONSHINY,
        m_settings.NOTIFICATION_SHINY,
        m_env.program_info(),
        enable_names, is_shiny(result.shiny_type),
        results(encounter),
        result.best_screenshot,
        &m_session_stats,
        enable_names ? &m_frequencies : nullptr
    );

    m_console.botbase().wait_for_all_requests();

    run_from_battle(m_env, m_console, exit_battle_time);
}


bool StandardEncounterHandler::handle_standard_encounter(const DoublesShinyDetection& result){
    if (result.shiny_type == ShinyType::UNKNOWN){
        m_console.log("Unable to determine result of battle.", COLOR_RED);
        m_session_stats.add_error();
        m_consecutive_failures++;
        if (m_consecutive_failures >= 3){
            PA_THROW_StringException("3 consecutive failed encounter detections.");
        }
        return false;
    }
    m_consecutive_failures = 0;

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

    bool enable_names = m_language != Language::None;
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
        enable_names ? &m_frequencies : nullptr
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
        m_console.log("Unable to determine result of battle.", COLOR_RED);
        m_session_stats.add_error();
        m_consecutive_failures++;
        if (m_consecutive_failures >= 3){
            PA_THROW_StringException("3 consecutive failed encounter detections.");
        }
        return false;
    }
    m_consecutive_failures = 0;

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

    bool enable_names = m_language != Language::None;
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
        enable_names ? &m_frequencies : nullptr
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
