/*  Shiny Hunt - Legendary Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Notification.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ShinySymbolDetector.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA_ShinyHunt-LakeTrio.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;


ShinyHuntLakeTrio_Descriptor::ShinyHuntLakeTrio_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:ShinyHunt-LakeTrio",
        STRING_POKEMON + " LA", "Shiny Hunt - Lake Trio",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/ShinyHunt-LakeTrio.md",
        "Shiny hunt the lake trio legendaries.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
std::unique_ptr<StatsTracker> ShinyHuntLakeTrio_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new PokemonSwSh::ShinyHuntTracker(false));
}


ShinyHuntLakeTrio::ShinyHuntLakeTrio()
    : VIDEO_ON_SHINY(
        "<b>Video Capture:</b><br>Take a video of the encounter if it is shiny.",
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , NOTIFICATION_NONSHINY(
        "Non-Shiny Encounter",
        true, false,
        {"Notifs"},
        std::chrono::seconds(3600)
    )
    , NOTIFICATION_SHINY(
        "Shiny Encounter",
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
//    PA_ADD_OPTION(GO_HOME_WHEN_DONE);

    PA_ADD_OPTION(LANGUAGE);

//    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);
}



std::set<std::string> read_name(
    Logger& logger,
    Language language,
    const ImageViewRGB32& screen, const ImageFloatBox& box
){
    if (language == Language::None){
        return {};
    }


    ImageViewRGB32 image = extract_box_reference(screen, box);
    OCR::StringMatchResult result = Pokemon::PokemonNameReader::instance().read_substring(
        logger, language, image,
        OCR::WHITE_TEXT_FILTERS()
    );
    result.clear_beyond_log10p(Pokemon::PokemonNameReader::MAX_LOG10P);

    std::set<std::string> ret;
    if (result.results.empty()){
        dump_image(
            logger, ProgramInfo(),
            "NameOCR-" + language_data(language).code,
            screen
        );
    }else{
        for (const auto& item : result.results){
            ret.insert(item.second.token);
        }
    }
    return ret;
}



void ShinyHuntLakeTrio::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    PokemonSwSh::ShinyHuntTracker& stats = env.current_stats<PokemonSwSh::ShinyHuntTracker>();


    //  Connect the controller.
    pbf_press_button(context, BUTTON_B, 5, 5);

    size_t consecutive_errors = 0;

    bool reset = false;
    while (true){
        env.update_stats();

        if (reset){
            go_home(env.console, context);
            if (!reset_game_from_home(env, env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST)){
                stats.add_error();
                continue;
            }
        }
        reset = true;

        env.console.log("Entering cave...");
        pbf_move_left_joystick(context, 160, 0, 50, 0);
        pbf_move_left_joystick(context, 96, 0, 50, 0);
        pbf_move_left_joystick(context, 160, 0, 50, 0);
        pbf_move_left_joystick(context, 96, 0, 50, 0);
        pbf_mash_button(context, BUTTON_A, 125);
        context.wait_for_all_requests();


        env.console.log("Waiting for a target to appear...");
        {
            ArcDetector arcs;
            WhiteObjectWatcher watcher(
                env.console,
                {0, 0, 1, 1},
                {{arcs, true}}
            );
            int ret = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_B, 60 * TICKS_PER_SECOND);
                },
                {{watcher}}
            );
            if (ret < 0){
                VideoSnapshot screen = env.console.video().snapshot();
                env.log("No encounter detected after 60 seconds.", COLOR_RED);
                stats.add_error();
                dump_image(
                    env.logger(), ProgramInfo(),
                    "NoEncounter",
                    screen
                );
                send_program_recoverable_error_notification(
                    env, NOTIFICATION_ERROR_RECOVERABLE,
                    "No encounter detected after 60 seconds."
                );
                consecutive_errors++;
                if (consecutive_errors >= 3){
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Failed to detect an encounter 3 times in the row.",
                        env.console
                    );
                }
                continue;
            }
        }

        env.console.log("Locking on and searching for shiny symbol...");
        {
            ArcDetector arcs;
            WhiteObjectWatcher watcher(
                env.console,
                {0, 0, 1, 1},
                {{arcs, false}}
            );
            ShinySymbolWaiter shiny_symbol(env.console, SHINY_SYMBOL_BOX_BOTTOM);
            int ret = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context){
                    pbf_press_button(context, BUTTON_ZL, 3 * TICKS_PER_SECOND, 0);
                },
                {
                    {watcher},
                    {shiny_symbol},
                }
            );
            VideoSnapshot screen = env.console.video().snapshot();
            std::set<std::string> slugs = read_name(env.logger(), LANGUAGE, screen, {0.11, 0.868, 0.135, 0.043});

            if (ret < 0){
                stats.add_non_shiny();
                env.log("Not shiny.", COLOR_PURPLE);
                send_encounter_notification(
                    env,
                    NOTIFICATION_NONSHINY,
                    NOTIFICATION_SHINY,
                    true, false, {{std::move(slugs), ShinyType::NOT_SHINY}}, std::nan(""),
                    screen
                );
            }else{
                stats.add_unknown_shiny();
                env.log("Detected Shiny!", COLOR_BLUE);
                send_encounter_notification(
                    env,
                    NOTIFICATION_NONSHINY,
                    NOTIFICATION_SHINY,
                    true, true, {{std::move(slugs), ShinyType::UNKNOWN_SHINY}}, std::nan(""),
                    screen
                );
                if (VIDEO_ON_SHINY){
//                    pbf_wait(context, 5 * TICKS_PER_SECOND);
                    pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 0);
                }
                break;
            }
        }

    }

    env.update_stats();
    pbf_press_button(context, BUTTON_HOME, 80ms, GameSettings::instance().GAME_TO_HOME_DELAY0);

//    GO_HOME_WHEN_DONE.run_end_of_program(context);
}




}
}
}
