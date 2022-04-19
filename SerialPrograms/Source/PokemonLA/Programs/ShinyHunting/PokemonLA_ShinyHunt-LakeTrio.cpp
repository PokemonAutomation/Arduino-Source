/*  Shiny Hunt - Legendary Reset
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/OCR/OCR_Filtering.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
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
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:ShinyHunt-LakeTrio",
        STRING_POKEMON + " LA", "Shiny Hunt - Lake Trio",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/ShinyHunt-LakeTrio.md",
        "Shiny hunt the lake trio legendaries.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


ShinyHuntLakeTrio::ShinyHuntLakeTrio(const ShinyHuntLakeTrio_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
//    , GO_HOME_WHEN_DONE(false)
    , VIDEO_ON_SHINY(
        "<b>Video Capture:</b><br>Take a video of the encounter if it is shiny.",
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


std::unique_ptr<StatsTracker> ShinyHuntLakeTrio::make_stats() const{
    return std::unique_ptr<StatsTracker>(new PokemonSwSh::ShinyHuntTracker(false));
}



std::set<std::string> read_name(
    LoggerQt& logger,
    Language language,
    const QImage& screen, const ImageFloatBox& box
){
    if (language == Language::None){
        return {};
    }

    QImage image = extract_box_copy(screen, box);
    OCR::filter_smart(image);

    std::set<std::string> ret;

    OCR::StringMatchResult result = PokemonNameReader::instance().read_substring(logger, language, image);
    if (result.results.empty()){
        dump_image(
            logger, ProgramInfo(),
            QString::fromStdString("NameOCR-" + language_data(language).code),
            screen
        );
    }else{
        for (const auto& item : result.results){
            ret.insert(item.second.token);
        }
    }
    return ret;
}



void ShinyHuntLakeTrio::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    PokemonSwSh::ShinyHuntTracker& stats = env.stats<PokemonSwSh::ShinyHuntTracker>();


    //  Connect the controller.
    pbf_press_button(context, BUTTON_B, 5, 5);

    size_t consecutive_errors = 0;

    bool reset = false;
    while (true){
        env.update_stats();

        if (reset){
            pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY);
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
            int ret = run_until(
                env.console, context,
                [=](BotBaseContext& context){
                    pbf_mash_button(context, BUTTON_B, 60 * TICKS_PER_SECOND);
                },
                {{watcher}}
            );
            if (ret < 0){
                QImage screen = env.console.video().snapshot();
                env.log("No encounter detected after 60 seconds.", COLOR_RED);
                stats.add_error();
                dump_image(
                    env.logger(), ProgramInfo(),
                    QString::fromStdString("NoEncounter"),
                    screen
                );
                send_program_recoverable_error_notification(
                    env.logger(),
                    NOTIFICATION_ERROR_RECOVERABLE,
                    env.program_info(),
                    "No encounter detected after 60 seconds.",
                    stats.to_str()
                );
                consecutive_errors++;
                if (consecutive_errors >= 3){
                    throw OperationFailedException(env.console, "Failed to detect an encounter 3 times in the row.");
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
            int ret = run_until(
                env.console, context,
                [=](BotBaseContext& context){
                    pbf_press_button(context, BUTTON_ZL, 3 * TICKS_PER_SECOND, 0);
                },
                {
                    {watcher},
                    {shiny_symbol},
                }
            );
            QImage screen = env.console.video().snapshot();
            std::set<std::string> slugs = read_name(env.logger(), LANGUAGE, screen, {0.11, 0.868, 0.135, 0.043});

            if (ret < 0){
                stats.add_non_shiny();
                env.log("Not shiny.", COLOR_PURPLE);
                send_encounter_notification(
                    env.console,
                    NOTIFICATION_NONSHINY,
                    NOTIFICATION_SHINY,
                    env.program_info(),
                    true, false, {{std::move(slugs), ShinyType::NOT_SHINY}},
                    screen,
                    &stats
                );
            }else{
                stats.add_unknown_shiny();
                env.log("Detected Shiny!", COLOR_BLUE);
                send_encounter_notification(
                    env.console,
                    NOTIFICATION_NONSHINY,
                    NOTIFICATION_SHINY,
                    env.program_info(),
                    true, true, {{std::move(slugs), ShinyType::UNKNOWN_SHINY}},
                    screen,
                    &stats
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
    pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY);

//    GO_HOME_WHEN_DONE.run_end_of_program(context);
}




}
}
}
