/*  Encounter Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
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
    bool require_square,
    uint16_t exit_battle_time,
    bool take_video,
    bool run_from_everything
)
    : m_stats(stats)
    , m_env(env)
    , m_console(console)
    , m_require_square(require_square)
    , m_exit_battle_time(exit_battle_time)
    , m_take_video(take_video)
    , m_run_from_everything(run_from_everything)
{}

bool StandardEncounterTracker::run_away(){
#if 1
    InterruptableCommandSession commands(m_console);

    BlackScreenDetector black_screen_detector(m_console);
    black_screen_detector.register_command_stop(commands);

    AsyncVisualInferenceSession inference(m_env, m_console);
    inference += black_screen_detector;

    commands.run([=](const BotBaseContext& context){
        pbf_press_dpad(context, DPAD_UP, 10, 10);
        pbf_mash_button(context, BUTTON_A, TICKS_PER_SECOND);
        if (m_exit_battle_time > TICKS_PER_SECOND){
            pbf_mash_button(context, BUTTON_B, m_exit_battle_time - TICKS_PER_SECOND);
        }
        context.botbase().wait_for_all_requests();
    });
#else
    pbf_press_dpad(m_console, DPAD_UP, 10, 10);
    pbf_mash_button(m_console, BUTTON_A, TICKS_PER_SECOND);
    if (m_exit_battle_time > TICKS_PER_SECOND){
        pbf_mash_button(m_console, BUTTON_B, m_exit_battle_time - TICKS_PER_SECOND);
    }
#endif
    return true;
}

void StandardEncounterTracker::take_video(){
    if (m_take_video){
        pbf_wait(m_console, 5 * TICKS_PER_SECOND);
        pbf_press_button(m_console, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
    }
}

bool StandardEncounterTracker::process_result(ShinyDetection detection){
    bool stop = false;
    switch (detection){
    case ShinyDetection::NO_BATTLE_MENU:
        return false;

    case ShinyDetection::NOT_SHINY:
        m_stats.add_non_shiny();
        break;

    case ShinyDetection::STAR_SHINY:
        m_stats.add_star_shiny();
        take_video();
        stop = !m_require_square;
        break;

    case ShinyDetection::SQUARE_SHINY:
        m_stats.add_square_shiny();
        take_video();
        stop = true;
        break;

    case ShinyDetection::UNKNOWN_SHINY:
        m_stats.add_unknown_shiny();
        take_video();
        stop = true;
        break;

    }

    if (m_run_from_everything){
        stop = false;
    }

    if (!stop){
        run_away();
    }

    return stop;
}


}
}
}

