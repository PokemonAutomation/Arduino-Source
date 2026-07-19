/*  Daily Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "CommonFramework/Logging/Logger.h"
#include <chrono>
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip.h"
#include "Pokemon/Pokemon_Strings.h"
// #include "PokemonPokopia/Inference/PokemonPokopia_ButtonDetector.h"
// #include "PokemonPokopia/Inference/PokemonPokopia_MovesDetection.h"
// #include "PokemonPokopia/Inference/PokemonPokopia_PCDetection.h"
// #include "PokemonPokopia/Inference/PokemonPokopia_SettingsScreenDetector.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"

#include "PokemonPokopia/Programs/PokemonPokopia_PCNavigation.h"
#include "PokemonPokopia/Programs/PokemonPokopia_DailyFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonPokopia{

using namespace Pokemon;


DailyFarmer_Descriptor::DailyFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonPokopia:DailyFarmer",
        STRING_POKEMON + " Pokopia", "Daily Farmer",
        "Programs/PokemonPokopia/DailyFarmer.html",
        "Farm Stamps, Coins, and Recipes by time skipping",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
class DailyFarmer_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : skips(m_stats["Skips"])
        , stamps(m_stats["Stamps"])
        , recipes_purchased(m_stats["Recipes Purchased"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Skips");
        m_display_order.emplace_back("Stamps");
        m_display_order.emplace_back("Recipes Purchased");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& skips;
    std::atomic<uint64_t>& stamps;
    std::atomic<uint64_t>& recipes_purchased;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> DailyFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


DailyFarmer::DailyFarmer()
    : STOP_AFTER_CURRENT("Reset")
    , NUM_SKIPS(
        "<b>Number of Day Skips to Run:</b><br>"
        "Zero will run until 'Stop after Current Reset' is pressed or the program is manually stopped.",
        LockMode::UNLOCK_WHILE_RUNNING,
        500,
        0
    )
    , COLLECT_STAMPS(
        "<b>Collect Stamps:</b><br>"
        "Whether or not to collect stamps to farm coins",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    // , BUY_RECIPES(
    //     "<b>Buy Recipes:</b><br>"
    //     "Whether to buy recipes from the Cloud Island shop.",
    //     LockMode::UNLOCK_WHILE_RUNNING,
    //     false
    // )
    // , SPEND_LIMIT(
    //     "<b>Spend Limit:</b><br>"
    //     "The limit for how many Life Coins should remain after buying recipes.",
    //     LockMode::UNLOCK_WHILE_RUNNING,
    //     5000,
    //     0
    // )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(STOP_AFTER_CURRENT);
    PA_ADD_OPTION(NUM_SKIPS);
    PA_ADD_OPTION(COLLECT_STAMPS);
    // PA_ADD_OPTION(BUY_RECIPES);
    // PA_ADD_OPTION(SPEND_LIMIT);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void DailyFarmer::go_to_date_menu(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DateChangeWatcher date_change_watcher(env.console);

    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            for (int i = 0; i < 3; i++){
                go_home(env.console, context);
                home_to_date_time(env.console, context, true);
                pbf_press_button(context, BUTTON_A, 80ms, 1000ms);
                context.wait_for_all_requests();
            }
        },
        { date_change_watcher }
    );
    if (ret == 0){
        env.console.log("Successfully navigated to date change menu");
        return;
    }
    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "Failed to navigate to date change menu",
        env.console
    );
}

void DailyFarmer::date_skip(SingleSwitchProgramEnvironment& env, ProControllerContext& context, DateSkipMode mode){
    go_to_date_menu(env, context);

    DateReader date_reader(env.console);

    VideoSnapshot date_time_snapshot = env.console.video().snapshot();
    std::pair<DateFormat, DateTime> current_date = date_reader.read_date(env.logger(), date_time_snapshot);

    const std::chrono::year_month_day current_ymd{
        std::chrono::year(current_date.second.year), 
        std::chrono::month(current_date.second.month), 
        std::chrono::day(current_date.second.day)
    };
    const std::chrono::sys_days current_sys_days{current_ymd};
    const std::chrono::weekday current_weekday{current_sys_days};
    env.console.log("Current weekday: " + std::format("{:%A}", current_weekday));

    const int days_to_next_friday = current_weekday == std::chrono::Friday ? 7 : (std::chrono::Friday - current_weekday).count();
    const int days_to_increment = (mode == DateSkipMode::NEXT_FRIDAY) ? days_to_next_friday : 1;

    const std::chrono::year_month_day next_ymd{current_sys_days + std::chrono::days(days_to_increment)};
    env.console.log("Next date: " + std::format("{:%Y-%m-%d}", next_ymd));

    DateTime next_date(
        static_cast<uint16_t>(static_cast<int>(next_ymd.year())),
        static_cast<uint8_t>(static_cast<unsigned>(next_ymd.month())),
        static_cast<uint8_t>(static_cast<unsigned>(next_ymd.day())),
        current_date.second.hour,
        current_date.second.minute,
        current_date.second.second
    );
    date_reader.set_date(env.program_info(), env.console, context, next_date);
    pbf_press_button(context, BUTTON_A, 160ms, 240ms);
    context.wait_for_all_requests();

    pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
    resume_game_from_home(env.console, context, false);
}

void DailyFarmer::add_todays_stamp(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DailyFarmer_Descriptor::Stats& stats = env.current_stats<DailyFarmer_Descriptor::Stats>();
    // Technically this replacement shouldn't ever happen since the program should skip to Friday and redeem after 5 stamps
    SelectionArrowWatcher replace_stamp_selection_watcher(
        COLOR_YELLOW, &env.console.overlay(),
        SelectionArrowType::DOWN,
        STAMP_CARD_BOX
    );
    bool stamp_added = add_stamp(env.console, context, replace_stamp_selection_watcher);
    if (stamp_added){
        stats.stamps++;
        env.update_stats();
        env.console.log("Added today's stamp");
    }
    else {
        // If this is reached something went wrong but should be recoverable
        env.console.log("Failed to add today's stamp, exiting PC. Please report this as a bug");
    }
    exit_pc(env.console, context);
}

void DailyFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    DailyFarmer_Descriptor::Stats& stats = env.current_stats<DailyFarmer_Descriptor::Stats>();

    DeferredStopButtonOption::ResetOnExit reset_on_exit(STOP_AFTER_CURRENT);

    while (NUM_SKIPS == 0 || stats.skips < NUM_SKIPS){
        date_skip(env, context, DateSkipMode::NEXT_FRIDAY);
        env.console.log("Date skipped to the next Friday");

        // If stamps are to be collected, check the PC Friday to Tuesday and claim the stamp
        // Skip to next Friday after Tuesday to claim the stamp card with the full card bonus
        // Maximizing the number of full card bonuses is more efficient than resetting for Mews
        if (COLLECT_STAMPS){
            for (int i = 0; i < 5; i++){
                // Account for loading or event dialogs
                mash_until_overworld(env.console, context);
                stats.skips++;
                env.update_stats();
                if (stats.skips >= NUM_SKIPS){
                    break;
                }
                access_pc_from_overworld(env.console, context, true);
                add_todays_stamp(env, context);
                if (i < 4){
                    date_skip(env, context, DateSkipMode::TOMMORROW);
                }
            }
        }
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}



}
}
}
