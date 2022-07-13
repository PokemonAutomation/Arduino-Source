/*  Purple Beam Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_BeamSetter.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "PokemonSwSh_PurpleBeamFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


PurpleBeamFinder_Descriptor::PurpleBeamFinder_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:PurpleBeamFinder",
        STRING_POKEMON + " SwSh", "Purple Beam Finder",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/PurpleBeamFinder.md",
        "Automatically reset for a purple beam.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



PurpleBeamFinder::PurpleBeamFinder(const PurpleBeamFinder_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , EXTRA_LINE(
        "<b>Extra Line:</b><br>(German has an extra line of text.)",
        false
    )
    , NOTIFICATION_RED_BEAM("Red Beam", false, false)
    , NOTIFICATION_PURPLE_BEAM("Purple Beam", true, true, ImageAttachmentMode::JPG)
    , NOTIFICATIONS({
        &NOTIFICATION_RED_BEAM,
        &NOTIFICATION_PURPLE_BEAM,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> Don't adjust these unless you're having problems.</font>"
    )
    , SAVE_SCREENSHOT(
        "<b>Screenshot Purple Beams:</b> (for debugging purposes)",
        false
    )
    , TIMEOUT_DELAY(
        "<b>Timeout Delay:</b><br>Reset if no beam is detected after this long.",
        "2 * TICKS_PER_SECOND"
    )
//    , MAX_STDDEV(
//        "<b>Maximum Standard Deviation:</b><br>Range: 0 - 768",
//        10, 0, 768
//    )
    , MIN_BRIGHTNESS(
        "<b>Minimum Brightness:</b><br>Range: 0 - 768",
        500, 0, 768
    )
    , MIN_EUCLIDEAN(
        "<b>Minimum Euclidean Distance:</b><br>Range: 0 - 443",
        15, 0, 443
    )
    , MIN_DELTA_STDDEV_RATIO(
        "<b>Minimum Delta/Stddev Ratio:</b>",
        5.0, 0
    )
    , MIN_SIGMA_STDDEV_RATIO(
        "<b>Minimum Sigma/Stddev Ratio:</b>",
        5.0, 0
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(EXTRA_LINE);
    PA_ADD_OPTION(NOTIFICATIONS);
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_STATIC(m_advanced_options);
        PA_ADD_OPTION(TIMEOUT_DELAY);
        PA_ADD_OPTION(MIN_BRIGHTNESS);
        PA_ADD_OPTION(MIN_EUCLIDEAN);
        PA_ADD_OPTION(MIN_DELTA_STDDEV_RATIO);
        PA_ADD_OPTION(MIN_SIGMA_STDDEV_RATIO);
    }
}




struct PurpleBeamFinder::Stats : public StatsTracker{
    Stats()
        : attempts(m_stats["Attempts"])
        , timeouts(m_stats["Timeouts"])
        , red_detected(m_stats["Red Detected"])
        , red_presumed(m_stats["Red Presumed"])
        , red(m_stats["Red"])
        , purple(m_stats["Purple"])
    {
        m_display_order.emplace_back(Stat("Attempts"));
        m_display_order.emplace_back(Stat("Timeouts"));
//        m_display_order.emplace_back(Stat("Red Detected"));
//        m_display_order.emplace_back(Stat("Red Presumed"));
        m_display_order.emplace_back(Stat("Red"));
        m_display_order.emplace_back(Stat("Purple"));
        m_aliases["Red Detected"] = "Red";
        m_aliases["Red Presumed"] = "Red";
    }
    std::atomic<uint64_t>& attempts;
    std::atomic<uint64_t>& timeouts;
    std::atomic<uint64_t>& red_detected;
    std::atomic<uint64_t>& red_presumed;
    std::atomic<uint64_t>& red;
    std::atomic<uint64_t>& purple;
};
std::unique_ptr<StatsTracker> PurpleBeamFinder::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




void PurpleBeamFinder::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(context);
        resume_game_front_of_den_nowatts(context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
        pbf_mash_button(context, BUTTON_B, 100);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }
    context.wait_for_all_requests();


    Stats& stats = env.current_stats<Stats>();


    bool exit = false;
    while (true){
        //  Talk to den.
        pbf_press_button(context, BUTTON_A, 10, 450);
        if (EXTRA_LINE){
            pbf_press_button(context, BUTTON_A, 10, 300);
        }
        pbf_press_button(context, BUTTON_A, 10, 300);
        context.wait_for_all_requests();

        BeamSetter::Detection detection;
        {
            BeamSetter setter(env, env.console, context);
            detection = setter.run(
                SAVE_SCREENSHOT,
                TIMEOUT_DELAY,
                MIN_BRIGHTNESS,
                MIN_EUCLIDEAN,
                MIN_DELTA_STDDEV_RATIO,
                MIN_SIGMA_STDDEV_RATIO
            );
            stats.attempts++;
        }
        switch (detection){
        case BeamSetter::NO_DETECTION:
            stats.timeouts++;
            break;
        case BeamSetter::RED_DETECTED:
            stats.red_detected++;
            break;
        case BeamSetter::RED_ASSUMED:
            stats.red_presumed++;
            break;
        case BeamSetter::PURPLE:
            stats.purple++;
            exit = true;
            break;
        }
        env.update_stats();
        if (exit){
            break;
        }

        send_program_status_notification(env, NOTIFICATION_RED_BEAM, "Red Beam...");

        pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
        reset_game_from_home_with_inference(
            env.console, context,
            ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
        );
    }

    context.wait_for(std::chrono::seconds(2));
    send_program_finished_notification(
        env, NOTIFICATION_PURPLE_BEAM,
        "Found a purple beam!",
        env.console.video().snapshot()
    );
    while (true){
        pbf_press_button(context, BUTTON_B, 20, 20);
        pbf_press_button(context, BUTTON_LCLICK, 20, 20);
    }
}





}
}
}

