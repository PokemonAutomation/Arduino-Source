/*  BDSP Starter Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_EndBattleDetector.h"
#include "PokemonBDSP_StarterNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


const Milliseconds AFTER_BRIEFCASE = 4000ms;
const Milliseconds PRE_BRIEFCASE_DURATION = 18200ms;
const Milliseconds PROMPT_ANIMATION = 1000ms;
const Milliseconds SCHEDULING_SLACK = 100ms;


std::vector<BdspEyeTemplate> lake_eye_templates(BdspPlayerModel model){
    return {
        model == BdspPlayerModel::Model1
            ? BdspEyeTemplate{"PlayerEye-Lake-Model1.png",
                {0.5047, 0.4472, 0.0214, 0.0380}, "Player"}
            : BdspEyeTemplate{"PlayerEye-Lake-Model4.png",
                {0.5042, 0.4528, 0.0208, 0.0370}, "Player"},
        BdspEyeTemplate{"BarryEye-Lake.png", {0.5521, 0.4528, 0.0214, 0.0380}, "Barry"},
    };
}


uint8_t starter_cursor_steps(BdspStarter starter){
    switch (starter){
    case BdspStarter::Turtwig:  return 0;
    case BdspStarter::Chimchar: return 1;
    case BdspStarter::Piplup:   return 2;
    }
    return 0;
}

const char* starter_slug(BdspStarter starter){
    switch (starter){
    case BdspStarter::Turtwig:  return "turtwig";
    case BdspStarter::Chimchar: return "chimchar";
    case BdspStarter::Piplup:   return "piplup";
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown starter.");
}


void navigate_to_lake_blinks(Logger& logger, ProControllerContext& context){
    logger.log("Walking into the lake, then advancing to the lake blink position.");
    pbf_move_left_joystick(context, {0.0, 1.0}, 1600ms, 0ms);

    pbf_wait(context, 1500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 4000ms);
    pbf_press_button(context, BUTTON_A, 100ms, 2000ms);
    pbf_press_button(context, BUTTON_A, 100ms, 1500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 1500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 1500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 1500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 2500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 1500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 1500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 1500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 1500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 5500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 4500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 0ms);
    context.wait_for_all_requests();
}

void navigate_to_pre_briefcase(Logger& logger, ProControllerContext& context){
    logger.log("Advancing to the pre-briefcase position.");

    //  Seven presses with a total duration equal to PRE_BRIEFCASE_DURATION (above)
    pbf_wait(context, 1500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 3500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 2000ms);
    pbf_press_button(context, BUTTON_A, 100ms, 1500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 1500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 5500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 2000ms);
    pbf_press_button(context, BUTTON_A, 100ms, 0ms);
    context.wait_for_all_requests();
}


const Milliseconds BRIEFCASE_PRESS_TO_STARLY_READY =
    80ms + AFTER_BRIEFCASE + 100ms;
const Milliseconds SELECT_PRESS_TO_CONFIRM_READY =
    80ms + PROMPT_ANIMATION + 100ms + 200ms;

static Milliseconds starly_press_to_select_ready(BdspStarter starter){
    return 80ms + 5500ms + 100ms + 1200ms
        + starter_cursor_steps(starter) * (100ms + 600ms);
}


//  The whole blind run, from the briefcase press to the confirming press.
bool issue_starter_sequence(
    ProControllerContext& context,
    BdspStarter starter,
    double starly_seconds,
    double select_seconds,
    double confirm_seconds,
    std::string& failure_reason
){
    auto to_ms = [](double seconds){
        return std::chrono::duration_cast<Milliseconds>(
            std::chrono::duration<double>(seconds)
        );
    };

    //  Where the run stands after each fixed stretch, measured from the moment the
    //  briefcase press is issued.
    Milliseconds starly_at = to_ms(starly_seconds);
    Milliseconds select_at = to_ms(select_seconds);
    Milliseconds confirm_at = to_ms(confirm_seconds);
    Milliseconds after_dialog = BRIEFCASE_PRESS_TO_STARLY_READY;
    Milliseconds after_starly = starly_at + starly_press_to_select_ready(starter);
    Milliseconds after_select = select_at + SELECT_PRESS_TO_CONFIRM_READY;

    auto too_tight = [&](const char* what, Milliseconds needed, Milliseconds have){
        failure_reason = std::string(what) + " is scheduled "
            + std::to_string((needed - have).count())
            + " ms before the navigation ahead of it can finish";
        return false;
    };
    if (starly_at < after_dialog){
        return too_tight("the Starly press", after_dialog, starly_at);
    }
    if (select_at < after_starly){
        return too_tight("the selecting press", after_starly, select_at);
    }
    if (confirm_at < after_select){
        return too_tight("the confirming press", after_select, confirm_at);
    }

    //  The briefcase press
    pbf_press_button(context, BUTTON_A, 80ms, 0ms);

    //  Past the briefcase dialogue
    pbf_wait(context, AFTER_BRIEFCASE);
    pbf_press_button(context, BUTTON_A, 100ms, 0ms);

    //  The Starly press
    pbf_wait(context, starly_at - after_dialog);
    pbf_press_button(context, BUTTON_A, 80ms, 0ms);

    //  Open the briefcase and hover over the chosen ball
    pbf_wait(context, 5500ms);
    pbf_press_button(context, BUTTON_A, 100ms, 0ms);
    pbf_wait(context, 1200ms);
    for (uint8_t c = 0; c < starter_cursor_steps(starter); c++){
        pbf_press_dpad(context, DPAD_RIGHT, 100ms, 600ms);
    }

    //  The selecting press, then the cursor onto "Yes"
    pbf_wait(context, select_at - after_starly);
    pbf_press_button(context, BUTTON_A, 80ms, 0ms);
    pbf_wait(context, PROMPT_ANIMATION);
    pbf_press_dpad(context, DPAD_UP, 100ms, 200ms);

    //  The confirming press
    pbf_wait(context, confirm_at - after_select);
    pbf_press_button(context, BUTTON_A, 80ms, 0ms);

    //  battle start
    pbf_wait(context, 3000ms);
    pbf_press_button(context, BUTTON_A, 100ms, 0ms);

    context.wait_for_all_requests();
    return true;
}


//  Where the blind run leaves off. Feedback-driven, since nothing past the
//  confirming press is timed against the RNG.
bool clear_starter_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    env.log("Mashing A through the battle.");
    EndBattleWatcher battle_over;
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, 180s);
        },
        {{battle_over}}
    );
    if (ret < 0){
        env.log("The battle never ended.", COLOR_RED);
        return false;
    }
    env.log("Battle over.");
    return true;
}


double seconds_to_move_to_starter(BdspStarter starter){
    return std::chrono::duration_cast<std::chrono::duration<double>>(
        starly_press_to_select_ready(starter) - 80ms
    ).count();
}

// kept as short as the prompt animation allows to avoid extra blinks from the starter
double seconds_from_select_to_confirm(){
    return std::chrono::duration_cast<std::chrono::duration<double>>(
        SELECT_PRESS_TO_CONFIRM_READY + SCHEDULING_SLACK
    ).count();
}


double seconds_to_pre_briefcase(){
    return std::chrono::duration_cast<std::chrono::duration<double>>(
        PRE_BRIEFCASE_DURATION
    ).count();
}


double seconds_from_briefcase_to_starly_ready(){
    return std::chrono::duration_cast<std::chrono::duration<double>>(
        BRIEFCASE_PRESS_TO_STARLY_READY
    ).count();
}




}
}
}
