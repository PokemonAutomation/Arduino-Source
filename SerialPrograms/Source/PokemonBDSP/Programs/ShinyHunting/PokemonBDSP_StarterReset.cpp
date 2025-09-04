/*  Starter Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonBDSP/Resources/PokemonBDSP_NameDatabase.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameEntry.h"
#include "PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.h"
#include "PokemonBDSP/Inference/ShinyDetection/PokemonBDSP_ShinyEncounterDetector.h"
#include "PokemonBDSP_StarterReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


StarterReset_Descriptor::StarterReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:StarterReset",
        STRING_POKEMON + " BDSP", "Starter Reset",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/StarterReset.md",
        "Shiny hunt your starter " + STRING_POKEMON + ".",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct StarterReset_Descriptor::Stats : public PokemonSwSh::ShinyHuntTracker{
    Stats()
        : ShinyHuntTracker(false)
        , m_shiny_starly(m_stats["Shiny Starly"])
    {
        m_display_order.emplace_back("Shiny Starly", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& m_shiny_starly;
};
std::unique_ptr<StatsTracker> StarterReset_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


StarterReset::StarterReset()
    : GO_HOME_WHEN_DONE(false)
    , STARTER_DATABASE(make_name_database({"turtwig", "chimchar", "piplup"}))
    , STARTER(
        "<b>Starter:</b>",
        STARTER_DATABASE,
        LockMode::LOCK_WHILE_RUNNING,
        "turtwig"
    )
    , USE_SOUND_DETECTION(
        "<b>Use Sound Detection:</b><br>Use sound to improve shiny detection.<br>"
        "<b>Make sure you have correct audio input set.</b>",
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , VIDEO_ON_SHINY(
        "<b>Video Capture:</b><br>Take a video of the encounter if it is shiny.",
        LockMode::LOCK_WHILE_RUNNING,
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
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
//    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(STARTER);
    PA_ADD_OPTION(USE_SOUND_DETECTION);
    PA_ADD_OPTION(VIDEO_ON_SHINY);
    PA_ADD_OPTION(NOTIFICATIONS);
}





void StarterReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StarterReset_Descriptor::Stats& stats = env.current_stats<StarterReset_Descriptor::Stats>();

    std::shared_ptr<const ImageRGB32> briefcase = std::make_shared<const ImageRGB32>(RESOURCE_PATH() + "PokemonBDSP/StarterBriefcase.png");

    //  Connect the controller.
    pbf_press_button(context, BUTTON_B, 5, 5);

    size_t consecutive_failures = 0;

    bool reset = false;
    while (true){
        env.update_stats();

        if (consecutive_failures >= 3){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed 3 times in the row.",
                env.console
            );
        }

        if (reset){
            go_home(env.console, context);
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
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
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
            dump_image(env.console, env.program_info(), env.console, "Briefcase");
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
            env, env.console, context,
            result_wild, result_own,
            NOTIFICATION_ERROR_RECOVERABLE,
            YOUR_POKEMON,
            std::chrono::seconds(30),
            USE_SOUND_DETECTION
        );

//        if (result_wild.shiny_type == ShinyType::UNKNOWN || result_own.shiny_type == ShinyType::UNKNOWN){
        if (result_own.shiny_type == ShinyType::UNKNOWN){
            stats.add_error();
            consecutive_failures++;
            dump_image(env.console, env.program_info(), env.console, "UnknownShinyDetection");
        }else{
            consecutive_failures = 0;
        }

        bool wild_shiny = is_likely_shiny(result_wild.shiny_type);
        if (wild_shiny){
            stats.m_shiny_starly++;
            send_encounter_notification(
                env,
                NOTIFICATION_NONSHINY,
                NOTIFICATION_SHINY,
                true, true, {{{"starly"}, ShinyType::UNKNOWN_SHINY}}, result_wild.alpha,
                result_wild.get_best_screenshot()
            );
            pbf_wait(context, 5 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
        }

        bool your_shiny = is_likely_shiny(result_own.shiny_type);
        if (your_shiny){
            stats.add_unknown_shiny();
            send_encounter_notification(
                env,
                NOTIFICATION_NONSHINY,
                NOTIFICATION_SHINY,
                true, true, {{{starter}, ShinyType::UNKNOWN_SHINY}}, result_own.alpha,
                result_own.get_best_screenshot()
            );
            pbf_wait(context, 5 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
            break;
        }else{
            stats.add_non_shiny();
            send_encounter_notification(
                env,
                NOTIFICATION_NONSHINY,
                NOTIFICATION_SHINY,
                true, false, {{{starter}, ShinyType::NOT_SHINY}}, result_own.alpha,
                result_own.get_best_screenshot()
            );
        }
    }

    env.update_stats();
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}





}
}
}
