/*  Pokedex Recommendation Finder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_NeutralDateSkip.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Resources/PokemonSwSh_NameDatabase.h"
#include "PokemonSwSh_DexRecFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;



DexRecFinder_Descriptor::DexRecFinder_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:DexRecFinder",
        STRING_POKEMON + " SwSh", "Dex Rec Finder",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/DexRecFinder.md",
        "Search for a " + STRING_POKEDEX + " recommendation by date-spamming.",
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::OPTIONAL_,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct DexRecFinder_Descriptor::Stats : public StatsTracker{
    Stats()
        : attempts(m_stats["Attempts"])
        , errors(m_stats["Read Errors"])
        , excluded(m_stats["Excluded"])
        , matches(m_stats["Matches"])
    {
        m_display_order.emplace_back(Stat("Attempts"));
        m_display_order.emplace_back(Stat("Read Errors"));
        m_display_order.emplace_back(Stat("Excluded"));
        m_display_order.emplace_back(Stat("Matches"));
    }

    std::atomic<uint64_t>& attempts;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& excluded;
    std::atomic<uint64_t>& matches;
};
std::unique_ptr<StatsTracker> DexRecFinder_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



DexRecFilters::DexRecFilters()
    : GroupOption(
        "Stop Automatically (requires video feedback)",
        LockMode::LOCK_WHILE_RUNNING,
        GroupOption::EnableMode::DEFAULT_ENABLED
    )
    , LANGUAGE(
        "<b>Game Language:</b><br>This needs to be set correctly for stop filters to work correctly.",
        PokemonNameReader::instance().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , DESIRED(
        "<b>Desired " + STRING_POKEMON + ":</b><br>Stop when it finds this " + STRING_POKEMON + ". Requires the language be set.",
        COMBINED_DEX_NAMES(),
        LockMode::LOCK_WHILE_RUNNING,
        "ralts"
    )
    , EXCLUSIONS(
        "<b>Exclusions:</b><br>Do not stop on these " + STRING_POKEMON + " even if the desired " + STRING_POKEMON + " is found. "
        "Use this to avoid dex recs that include other " + STRING_POKEMON + " in the spawn pool you don't want.",
        STRING_POKEMON, COMBINED_DEX_NAMES(), "grubbin"
    )
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(DESIRED);
    PA_ADD_OPTION(EXCLUSIONS);
}



DexRecFinder::DexRecFinder()
    : GO_HOME_WHEN_DONE(false)
    , VIEW_TIME0(
        "<b>View Time:</b><br>View the " + STRING_POKEDEX + " for this long before continuing.",
        LockMode::LOCK_WHILE_RUNNING,
        "2000 ms"
    )
    , NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true, ImageAttachmentMode::JPG)
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , ENTER_POKEDEX_TIME0(
        "<b>Enter " + STRING_POKEDEX + " Time:</b><br>Wait this long for the " + STRING_POKEDEX + " to open.",
        LockMode::LOCK_WHILE_RUNNING,
        "3000 ms"
    )
    , BACK_OUT_TIME0(
        "<b>Back Out Time:</b><br>Mash B for this long to return to the overworld.",
        LockMode::LOCK_WHILE_RUNNING,
        "3000 ms"
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);

    PA_ADD_OPTION(FILTERS);
    PA_ADD_OPTION(VIEW_TIME0);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(ENTER_POKEDEX_TIME0);
    PA_ADD_OPTION(BACK_OUT_TIME0);
}


void DexRecFinder::read_line(
    bool& found,
    bool& excluded,
    bool& bad_read,
    Logger& logger,
    Language language,
    const ImageViewRGB32& frame,
    const ImageFloatBox& box,
    const std::set<std::string>& desired,
    const std::set<std::string>& exclusions
){
    ImageViewRGB32 image = extract_box_reference(frame, box);
    OCR::StringMatchResult result = PokemonNameReader::instance().read_substring(
        logger, language, image,
        OCR::BLACK_TEXT_FILTERS()
    );
    if (result.results.empty()){
        bad_read = true;
        return;
    }
    for (const auto& hit : result.results){
        if (desired.find(hit.second.token) != desired.end()){
            found = true;
        }
        if (exclusions.find(hit.second.token) != exclusions.end()){
            excluded = true;
        }
    }
}

void DexRecFinder::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
        pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);
    }

    std::set<std::string> desired;
    desired.insert(FILTERS.DESIRED.slug());

    std::set<std::string> exclusions;
    for (std::string& slug : FILTERS.EXCLUSIONS.all_slugs()){
        exclusions.insert(std::move(slug));
    }

    DexRecFinder_Descriptor::Stats& stats = env.current_stats<DexRecFinder_Descriptor::Stats>();

    while (true){
        home_to_date_time(env.console, context, true);
        neutral_date_skip(env.console, context);
        settings_to_enter_game(context, true);
        pbf_mash_button(context, BUTTON_B, 90);
        ssf_press_button(context, BUTTON_X, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0, 160ms);

        if (FILTERS.enabled()){
            context.wait_for_all_requests();
            OverlayBoxScope box0(env.console, ImageFloatBox(0.75, 0.531 + 0 * 0.1115, 0.18, 0.059));
            OverlayBoxScope box1(env.console, ImageFloatBox(0.75, 0.531 + 1 * 0.1115, 0.18, 0.059));
            OverlayBoxScope box2(env.console, ImageFloatBox(0.75, 0.531 + 2 * 0.1115, 0.18, 0.059));
            OverlayBoxScope box3(env.console, ImageFloatBox(0.75, 0.531 + 3 * 0.1115, 0.18, 0.059));
            pbf_press_button(context, BUTTON_A, 80ms, ENTER_POKEDEX_TIME0);
            context.wait_for_all_requests();

            VideoSnapshot frame = env.console.video().snapshot();
            bool found = false;
            bool excluded = false;
            bool bad_read = false;
            if (!frame){
                bad_read = true;
            }else{
                read_line(found, excluded, bad_read, env.console, FILTERS.LANGUAGE, frame, box0, desired, exclusions);
                read_line(found, excluded, bad_read, env.console, FILTERS.LANGUAGE, frame, box1, desired, exclusions);
                read_line(found, excluded, bad_read, env.console, FILTERS.LANGUAGE, frame, box2, desired, exclusions);
                read_line(found, excluded, bad_read, env.console, FILTERS.LANGUAGE, frame, box3, desired, exclusions);
            }

            stats.attempts++;
            if (found){
                if (excluded){
                    env.log("Found desired, but contains exclusion.", COLOR_BLUE);
                    stats.excluded++;
                }else{
                    env.log("Found a match!", COLOR_BLUE);
                    stats.matches++;
                    break;
                }
            }
            if (bad_read){
                env.log("Read Errors. Pausing for user to see.", COLOR_RED);
                stats.errors++;
                pbf_wait(context, VIEW_TIME0);
            }
        }else{
            stats.attempts++;
//            stats.errors++;
            pbf_press_button(context, BUTTON_A, 80ms, ENTER_POKEDEX_TIME0);
            pbf_wait(context, VIEW_TIME0);
        }
        env.update_stats();

        pbf_mash_button(context, BUTTON_B, BACK_OUT_TIME0);
        pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);
    }

    env.update_stats();
    send_program_finished_notification(
        env, NOTIFICATION_PROGRAM_FINISH,
        "Found a match!",
        env.console.video().snapshot(), false
    );
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}



}
}
}
