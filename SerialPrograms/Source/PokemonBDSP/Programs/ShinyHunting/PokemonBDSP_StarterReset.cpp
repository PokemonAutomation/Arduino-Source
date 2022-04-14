/*  Starter Reset
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Inference/ImageMatchDetector.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameEntry.h"
#include "PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.h"
#include "PokemonBDSP/Inference/ShinyDetection/PokemonBDSP_ShinyEncounterDetector.h"
#include "PokemonBDSP_StarterReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


StarterReset_Descriptor::StarterReset_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonBDSP:StarterReset",
        STRING_POKEMON + " BDSP", "Starter Reset",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/StarterReset.md",
        "Shiny hunt your starter " + STRING_POKEMON + ".",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


StarterReset::StarterReset(const StarterReset_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , STARTER(
        "<b>Starter:</b>",
        {"turtwig", "chimchar", "piplup"},
        "turtwig"
    )
    , VIDEO_ON_SHINY(
        "<b>Video Capture:</b><br>Take a video of the encounter if it is shiny.",
        true
    )
    , NOTIFICATION_NONSHINY(
        "Non-Shiny Starter",
        true, false,
        {"Notifs"},
        std::chrono::seconds(3600)
    )
    , NOTIFICATION_SHINY(
        "Shiny Starter",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , NOTIFICATIONS({
        &NOTIFICATION_NONSHINY,
        &NOTIFICATION_SHINY,
        &NOTIFICATION_ERROR_FATAL,
    })
{
//    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(STARTER);
    PA_ADD_OPTION(VIDEO_ON_SHINY);
    PA_ADD_OPTION(NOTIFICATIONS);
}



struct StarterReset::Stats : public PokemonSwSh::ShinyHuntTracker{
    Stats()
        : ShinyHuntTracker(false)
        , m_shiny_starly(m_stats["Shiny Starly"])
    {
        m_display_order.emplace_back("Shiny Starly", true);
    }
    std::atomic<uint64_t>& m_shiny_starly;
};
std::unique_ptr<StatsTracker> StarterReset::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




void StarterReset::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    Stats& stats = env.stats<Stats>();

    QImage briefcase(RESOURCE_PATH() + "PokemonBDSP/StarterBriefcase.png");

    //  Connect the controller.
    pbf_press_button(context, BUTTON_B, 5, 5);

    size_t consecutive_failures = 0;

    bool reset = false;
    while (true){
        env.update_stats();

        if (consecutive_failures >= 3){
            throw OperationFailedException(env.console, "Failed 3 times in the row.");
        }

        if (reset){
            pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY);
            if (!reset_game_from_home(env, env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST)){
                stats.add_error();
                consecutive_failures++;
                continue;
            }
        }
        reset = true;

        //  Enter the lake.
        pbf_move_left_joystick(context, 128, 0, TICKS_PER_SECOND, 0);

        //  Mash B until we see the briefcase.
        ImageMatchWatcher detector(briefcase, {0.5, 0.1, 0.5, 0.7}, 100, true);
        int ret = run_until(
            env.console, context,
            [](BotBaseContext& context){
                pbf_mash_button(context, BUTTON_B, 120 * TICKS_PER_SECOND);
            },
            {{detector}}
        );
        if (ret == 0){
            env.log("Detected briefcase!");
        }else{
            env.log("Timed out waiting for briefcase.", COLOR_RED);
            stats.add_error();
            consecutive_failures++;
            dump_image(env.logger(), env.program_info(), "Briefcase", env.console.video().snapshot());
            continue;
        }

        //  Wait for briefcase to fully open.
        env.log("Mashing B for briefcase to fully open.");
        pbf_mash_button(context, BUTTON_B, 2 * TICKS_PER_SECOND);

        //  Scroll to your starter.
        size_t scroll = 0;
        const std::string& starter = STARTER.slug();
        env.log("Scrolling to starter... " + starter);
        if (starter == "turtwig"){
            scroll = 0;
        }else if (starter == "chimchar"){
            scroll = 1;
        }else if (starter == "piplup"){
            scroll = 2;
        }
        for (size_t c = 0; c < scroll; c++){
            pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
        }

        //  Select starter.
        pbf_press_button(context, BUTTON_ZL, 20, 30);
        context.wait_for_all_requests();

        {
            SelectionArrowFinder selection_arrow(env.console, {0.50, 0.60, 0.35, 0.20}, COLOR_RED);
            ret = wait_until(
                env.console, context, std::chrono::seconds(3),
                {{selection_arrow}}
            );
            if (ret == 0){
                env.log("Detected selection prompt!");
            }else{
                env.log("Timed out waiting for selection prompt.", COLOR_RED);
                consecutive_failures++;
            }
            pbf_wait(context, 50);
            pbf_press_dpad(context, DPAD_UP, 10, 50);
            pbf_press_button(context, BUTTON_ZL, 10, 5 * TICKS_PER_SECOND);
            context.wait_for_all_requests();
        }

        //  Detect shiny.
        DoublesShinyDetection result_wild;
        ShinyDetectionResult result_own;
        detect_shiny_battle(
            env.console, context,
            result_wild, result_own,
            YOUR_POKEMON,
            std::chrono::seconds(30)
        );

//        if (result_wild.shiny_type == ShinyType::UNKNOWN || result_own.shiny_type == ShinyType::UNKNOWN){
        if (result_own.shiny_type == ShinyType::UNKNOWN){
            stats.add_error();
            consecutive_failures++;
            dump_image(env.logger(), env.program_info(), "UnknownShinyDetection", env.console.video().snapshot());
        }else{
            consecutive_failures = 0;
        }

        bool wild_shiny = is_likely_shiny(result_wild.shiny_type);
        if (wild_shiny){
            stats.m_shiny_starly++;
            send_encounter_notification(
                env.console,
                NOTIFICATION_NONSHINY,
                NOTIFICATION_SHINY,
                env.program_info(),
                true, true, {{{"starly"}, ShinyType::UNKNOWN_SHINY}},
                result_wild.best_screenshot,
                &stats
            );
        }else{
#if 0
            send_encounter_notification(
                env.console,
                NOTIFICATION_NONSHINY,
                NOTIFICATION_SHINY,
                env.program_info(),
                true, false, {{{"starly"}, ShinyType::NOT_SHINY}},
                result_wild.best_screenshot,
                &stats
            );
#endif
        }

        bool your_shiny = is_likely_shiny(result_own.shiny_type);
        if (your_shiny){
            stats.add_unknown_shiny();
            send_encounter_notification(
                env.console,
                NOTIFICATION_NONSHINY,
                NOTIFICATION_SHINY,
                env.program_info(),
                true, true, {{{starter}, ShinyType::UNKNOWN_SHINY}},
                result_own.best_screenshot,
                &stats
            );
            break;
        }else{
            stats.add_non_shiny();
            send_encounter_notification(
                env.console,
                NOTIFICATION_NONSHINY,
                NOTIFICATION_SHINY,
                env.program_info(),
                true, false, {{{starter}, ShinyType::NOT_SHINY}},
                result_own.best_screenshot,
                &stats
            );
        }

        if ((wild_shiny || your_shiny) && VIDEO_ON_SHINY){
            pbf_wait(context, 5 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
        }

    }

    env.update_stats();
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}





}
}
}
