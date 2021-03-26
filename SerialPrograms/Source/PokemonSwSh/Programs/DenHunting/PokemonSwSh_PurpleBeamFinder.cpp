/*  Purple Beam Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Clientside/PrettyPrint.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "PokemonSwSh/Inference/PokemonSwSh_BeamSetter.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "PokemonSwSh_PurpleBeamFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


PurpleBeamFinder::PurpleBeamFinder()
    : SingleSwitchProgram(
        FeedbackType::REQUIRED, PABotBaseLevel::PABOTBASE_12KB,
        "Purple Beam Finder",
        "SerialPrograms/PurpleBeamFinder.md",
        "Automatically reset for a purple beam."
    )
    , EXTRA_LINE(
        "<b>Extra Line:</b><br>(German has an extra line of text.)",
        false
    )
    , DETECTION_THRESHOLD(
        "<b>Red Beam Detection Threshold:</b>",
        0.02, 0.0, 1.0
    )
    , TIMEOUT_DELAY(
        "<b>Timeout Delay:</b><br>Reset if no beam is detected after this long.",
        "2 * TICKS_PER_SECOND"
    )
{
    m_options.emplace_back(&EXTRA_LINE, "EXTRA_LINE");
    m_options.emplace_back(&DETECTION_THRESHOLD, "DETECTION_THRESHOLD");
    m_options.emplace_back(&TIMEOUT_DELAY, "TIMEOUT_DELAY");
}

void PurpleBeamFinder::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home();

    resume_game_front_of_den_nowatts(TOLERATE_SYSTEM_UPDATE_MENU_SLOW);
    pbf_mash_button(BUTTON_B, 100);
    env.console.botbase().wait_for_all_requests();


    uint64_t attempts = 0;
    uint64_t red_detected = 0;
    uint64_t red_assumed = 0;
    uint64_t timed_out = 0;
    uint64_t purple = 0;


    bool exit = false;
    while (true){
        //  Talk to den.
        pbf_press_button(BUTTON_A, 10, 450);
        if (EXTRA_LINE){
            pbf_press_button(BUTTON_A, 10, 300);
        }
        pbf_press_button(BUTTON_A, 10, 300);
        env.console.botbase().wait_for_all_requests();

        BeamSetter::Detection detection;
        {
            BeamSetter setter(env.console, env.logger);
            detection = setter.run(env, env.console, DETECTION_THRESHOLD, TIMEOUT_DELAY);
            attempts++;
        }
        switch (detection){
        case BeamSetter::NO_DETECTION:
            timed_out++;
            break;
        case BeamSetter::RED_DETECTED:
            red_detected++;
            break;
        case BeamSetter::RED_ASSUMED:
            red_assumed++;
            break;
        case BeamSetter::PURPLE:
            purple++;
            exit = true;
            break;
        }
        {
            std::string status = "<b>Attempts: " + tostr_u_commas(attempts);
            status += " - Timeouts: " + tostr_u_commas(timed_out);
            status += " - Red Detected: " + tostr_u_commas(red_detected);
            status += " - Red Presumed: " + tostr_u_commas(red_assumed);
            status += " - Purple: " + tostr_u_commas(purple);
            status += "</br>";
            QString str = status.c_str();
            env.logger.log(str);
            env.set_status(str);
        }
        if (exit){
            break;
        }

        pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
        reset_game_from_home_with_inference(
            env, env.logger, env.console,
            TOLERATE_SYSTEM_UPDATE_MENU_SLOW
        );
    }


    while (true){
        pbf_press_button(BUTTON_B, 20, 20);
        pbf_press_button(BUTTON_LCLICK, 20, 20);
    }
}





}
}
}

