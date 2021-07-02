/*  Encounter Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/VisualInferenceSession.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "CommonFramework/Inference/VisualInferenceSession.h"
#include "PokemonSwSh_EncounterTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{




StandardEncounterTracker::StandardEncounterTracker(
    ShinyHuntTracker& stats,
    ProgramEnvironment& env,
    ConsoleHandle& console,
    Pokemon::PokemonNameReader* name_reader, Language language,
    bool require_square,
    uint16_t exit_battle_time,
    bool take_video,
    bool run_from_everything
)
    : m_shiny_stats_tracker(stats)
    , m_env(env)
    , m_console(console)
    , m_name_reader(name_reader)
    , m_language(language)
    , m_require_square(require_square)
    , m_exit_battle_time(exit_battle_time)
    , m_take_video(take_video)
    , m_run_from_everything(run_from_everything)
{}

bool StandardEncounterTracker::run_away(bool confirmed_encounter){
    //  Initiate the run-away.
    pbf_press_dpad(m_console, DPAD_UP, 10, 40);
    pbf_mash_button(m_console, BUTTON_A, 20);
    m_console.botbase().wait_for_all_requests();

    //  While we are running away, read the name. We do this in parallel
    //  to avoid slowing down the program.
    if (confirmed_encounter){
        read_name();
    }

    InterruptableCommandSession commands(m_console);

    BlackScreenDetector black_screen_detector(m_console);
    black_screen_detector.register_command_stop(commands);

    AsyncVisualInferenceSession inference(m_env, m_console);
    inference += black_screen_detector;

    commands.run([=](const BotBaseContext& context){
        pbf_mash_button(context, BUTTON_A, TICKS_PER_SECOND);
        if (m_exit_battle_time > TICKS_PER_SECOND){
            pbf_mash_button(context, BUTTON_B, m_exit_battle_time - TICKS_PER_SECOND);
        }
        context.botbase().wait_for_all_requests();
    });
    return true;
}

void StandardEncounterTracker::take_video(){
    if (m_take_video){
        pbf_wait(m_console, 1 * TICKS_PER_SECOND);
        m_console.botbase().wait_for_all_requests();
        read_name();
        pbf_wait(m_console, 4 * TICKS_PER_SECOND);
        pbf_press_button(m_console, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
    }
}
void StandardEncounterTracker::read_name(){
    if (m_name_reader == nullptr || m_language == Language::None){
        return;
    }

//    m_env.wait(std::chrono::milliseconds(100));

    InferenceBoxScope box(m_console, InferenceBox(0.76, 0.04, 0.15, 0.044));
    QImage frame = m_console.video().snapshot();
    frame = extract_box(frame, box);
//    OCR::binary_filter_black_text(frame);

    OCR::MatchResult result = m_name_reader->read_exact(m_language, frame);
    result.log(&m_env.logger());

    if (result.matched){
        m_encounter_stats += std::move(result.tokens);
    }else{
        m_encounter_stats += std::set<std::string>();
    }

    m_env.log(m_encounter_stats.dump_sorted_map());
}


bool StandardEncounterTracker::process_result(ShinyDetection detection){
    bool stop = false;
    switch (detection){
    case ShinyDetection::NO_BATTLE_MENU:
        return false;

    case ShinyDetection::NOT_SHINY:
        m_shiny_stats_tracker.add_non_shiny();
        break;

    case ShinyDetection::STAR_SHINY:
        m_shiny_stats_tracker.add_star_shiny();
        take_video();
        stop = !m_require_square;
        break;

    case ShinyDetection::SQUARE_SHINY:
        m_shiny_stats_tracker.add_square_shiny();
        take_video();
        stop = true;
        break;

    case ShinyDetection::UNKNOWN_SHINY:
        m_shiny_stats_tracker.add_unknown_shiny();
        take_video();
        stop = true;
        break;

    }

    if (m_run_from_everything){
        stop = false;
    }

    if (!stop){
        run_away(true);
    }

    return stop;
}


}
}
}

