/*  Item Printer RNG
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Qt/TimeQt.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
//#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Inference/NintendoSwitch_DateReader.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
//#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV_ItemPrinterRNG.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



static const EnumDatabase<ItemPrinterJobs>& ItemPrinterJobs_Database(){
    static const EnumDatabase<ItemPrinterJobs> database({
        {ItemPrinterJobs::Jobs_1, "1", "1 Job"},
        {ItemPrinterJobs::Jobs_5, "5", "5 Jobs"},
        {ItemPrinterJobs::Jobs_10, "10", "10 Jobs"},
    });
    return database;
}

ItemPrinterRngRow::~ItemPrinterRngRow(){
    chain.remove_listener(*this);
}
ItemPrinterRngRow::ItemPrinterRngRow()
    : chain(LockMode::UNLOCK_WHILE_RUNNING, false)
    , date(
        LockMode::UNLOCK_WHILE_RUNNING,
        DateTimeOption::DATE_HOUR_MIN_SEC,
        DateTime{2000, 1, 1, 0, 1, 0},
        DateTime{2060, 12, 31, 23, 59, 59},
        DateTime{2024, 4, 22, 13, 27, 9}
    )
    , jobs(
        ItemPrinterJobs_Database(),
        LockMode::UNLOCK_WHILE_RUNNING,
        ItemPrinterJobs::Jobs_1
    )
{
    PA_ADD_OPTION(chain);
    PA_ADD_OPTION(date);
    PA_ADD_OPTION(jobs);
    chain.add_listener(*this);
}
ItemPrinterRngRow::ItemPrinterRngRow(bool p_chain, const DateTime& p_date, ItemPrinterJobs p_jobs)
    : ItemPrinterRngRow()
{
    chain = p_chain;
    date.set(p_date);
    jobs.set(p_jobs);
}
ItemPrinterRngRowSnapshot ItemPrinterRngRow::snapshot() const{
    return ItemPrinterRngRowSnapshot{chain, date, jobs};
}
std::unique_ptr<EditableTableRow> ItemPrinterRngRow::clone() const{
    std::unique_ptr<ItemPrinterRngRow> ret(new ItemPrinterRngRow());
    ret->chain = (bool)chain;
    ret->date.set(date);
    ret->jobs.set(jobs);
    return ret;
}
void ItemPrinterRngRow::value_changed(){
    date.set_visibility(chain ? ConfigOptionState::DISABLED : ConfigOptionState::ENABLED);
}

ItemPrinterRngTable::ItemPrinterRngTable(std::string label)
    : EditableTableOption_t<ItemPrinterRngRow>(
        std::move(label),
        LockMode::UNLOCK_WHILE_RUNNING,
        make_defaults()
    )
{}
std::vector<std::string> ItemPrinterRngTable::make_header() const{
    return std::vector<std::string>{
        "Continue Previous?",
        "Date Seed",
        "Jobs to Print",
    };
}
std::vector<std::unique_ptr<EditableTableRow>> ItemPrinterRngTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(std::make_unique<ItemPrinterRngRow>(false, DateTime{2039, 3, 21, 12, 14, 42}, ItemPrinterJobs::Jobs_1));
    ret.emplace_back(std::make_unique<ItemPrinterRngRow>(true, DateTime{2016, 5, 20, 2, 11, 13}, ItemPrinterJobs::Jobs_10));
    return ret;
}




ItemPrinterRNG_Descriptor::ItemPrinterRNG_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:ItemPrinterRNG",
        Pokemon::STRING_POKEMON + " SV", "Item Printer RNG",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/ItemPrinterRNG.md",
        "Farm the Item Printer using RNG Manipulation.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

struct ItemPrinterRNG_Descriptor::Stats : public StatsTracker{
    Stats()
        : iterations(m_stats["Rounds"])
        , prints(m_stats["Prints"])
//        , total_jobs(m_stats["Total Jobs"])
        , seed_hits(m_stats["Seed Hits"])
        , seed_misses(m_stats["Seed Misses"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Rounds");
        m_display_order.emplace_back("Prints");
//        m_display_order.emplace_back("Total Jobs", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Seed Hits", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Seed Misses", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& iterations;
    std::atomic<uint64_t>& prints;
//    std::atomic<uint64_t>& total_jobs;
    std::atomic<uint64_t>& seed_hits;
    std::atomic<uint64_t>& seed_misses;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> ItemPrinterRNG_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

ItemPrinterRNG::~ItemPrinterRNG(){
    AUTO_MATERIAL_FARMING.remove_listener(*this);
}

ItemPrinterRNG::ItemPrinterRNG()
    : LANGUAGE(
        "<b>Game Language:</b>",
        PokemonSwSh::IV_READER().languages(),
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , TOTAL_ROUNDS(
        "<b>Total Rounds:</b><br>Iterate the rounds table this many times before stopping the program.",
        LockMode::UNLOCK_WHILE_RUNNING, 10
    )
#if 0
    , DATE0(
        "<b>Initial Date:</b><br>Date/Time for the first print. This one should setup the bonus print.",
        LockMode::UNLOCK_WHILE_RUNNING,
        DateTimeOption::DATE_HOUR_MIN_SEC,
        DateTime{2000, 1, 1, 0, 1, 0},
        DateTime{2060, 12, 31, 23, 59, 59},
        DateTime{2024, 4, 22, 13, 27, 9}
    )
    , DATE1(
        "<b>Second Date:</b><br>Date/Time for the second print. This one gets you the main items.",
        LockMode::UNLOCK_WHILE_RUNNING,
        DateTimeOption::DATE_HOUR_MIN_SEC,
        DateTime{2000, 1, 1, 0, 1, 0},
        DateTime{2060, 12, 31, 23, 59, 59},
        DateTime{2016, 5, 20, 2, 11, 13}
    )
#endif
    , TABLE(
        "<b>Rounds Table:</b><br>Run the following prints in order and repeat. "
        "Changes to this table take effect the next time the table starts from the beginning."
    )
    , DELAY_MILLIS(
        "<b>Delay (Milliseconds):</b><br>"
        "The delay from when you press A to when the game reads the date for the seed. "
        "For OLED Switches, this delay is about 750 milliseconds. For older Switches, this delay is closer to 1750 milliseconds. "
        "You will need to experiment to find the correct delay.",
        LockMode::UNLOCK_WHILE_RUNNING, 750,
        0, 10000
    )
    , GO_HOME_WHEN_DONE(false)
    , FIX_TIME_WHEN_DONE(
        "<b>Fix Time When Done:</b><br>Fix the time after the program finishes.",
        LockMode::UNLOCK_WHILE_RUNNING, true
    )
    , AUTO_MATERIAL_FARMING(
        "<b>Auto material gathering:</b><br>"
        "After using the item printer, automatically fly to North Province (Area 3) to farm materials, "
        "then fly back to keep using the item printer.",
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
    , MATERIAL_FARMER_OPTIONS(
        LANGUAGE, GO_HOME_WHEN_DONE, 
        NOTIFICATION_STATUS_UPDATE, NOTIFICATION_PROGRAM_FINISH,
        NOTIFICATION_ERROR_RECOVERABLE, NOTIFICATION_ERROR_FATAL
    )
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(TOTAL_ROUNDS);
//    PA_ADD_OPTION(DATE0);
//    PA_ADD_OPTION(DATE1);
    PA_ADD_OPTION(TABLE);
    PA_ADD_OPTION(DELAY_MILLIS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(FIX_TIME_WHEN_DONE);

    PA_ADD_OPTION(AUTO_MATERIAL_FARMING);
    PA_ADD_OPTION(MATERIAL_FARMER_OPTIONS);

    PA_ADD_OPTION(NOTIFICATIONS);

    ItemPrinterRNG::value_changed();
    AUTO_MATERIAL_FARMING.add_listener(*this);
}

void ItemPrinterRNG::value_changed(){
    ConfigOptionState state = AUTO_MATERIAL_FARMING ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN;
    MATERIAL_FARMER_OPTIONS.set_visibility(state);

}


void ItemPrinterRNG::run_print_at_date(
    SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    const DateTime& date, ItemPrinterJobs jobs
) const{
    ItemPrinterRNG_Descriptor::Stats& stats = env.current_stats<ItemPrinterRNG_Descriptor::Stats>();

    bool printed = false;
    bool overworld_seen = false;
    while (true){
        context.wait_for_all_requests();

        OverworldWatcher overworld(COLOR_BLUE);
        AdvanceDialogWatcher dialog(COLOR_RED);
        PromptDialogWatcher prompt(COLOR_GREEN);
        DateChangeWatcher date_reader;
        WhiteButtonWatcher material(COLOR_GREEN, WhiteButton::ButtonX, {0.63, 0.93, 0.17, 0.06});
        int ret = wait_until(
            env.console, context, std::chrono::seconds(120),
            {
                overworld,
                dialog,
                prompt,
                date_reader,
                material,
            }
        );
        switch (ret){
        case 0:
            overworld_seen = true;
            if (printed){
                env.log("Detected overworld... (unexpected)", COLOR_RED);
                return;
            }
            env.log("Detected overworld... Starting print.");
            pbf_press_button(context, BUTTON_A, 20, 30);
            continue;

        case 1:
            env.log("Detected advance dialog.");
            pbf_press_button(context, BUTTON_B, 20, 30);
            continue;

        case 2:{
            env.log("Detected prompt dialog.");
            pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY);
            home_to_date_time(context, true, false);
            pbf_press_button(context, BUTTON_A, 10, 30);
            context.wait_for_all_requests();
            continue;
        }
        case 3:{
            env.log("Detected date change.");
            uint16_t delay_mills = DELAY_MILLIS;

            //  This is the seed we intend to hit.
            uint64_t seed_epoch_millis = to_seconds_since_epoch(date) * 1000;
            seed_epoch_millis -= delay_mills;

            //  This is the time we intend to set the clock to.
            uint64_t clock_epoch = (seed_epoch_millis - 5000) / 1000;
            clock_epoch /= 60;  //  Round down to minute.
            clock_epoch *= 60;
            DateTime set_date = from_seconds_since_epoch(clock_epoch);

            std::chrono::milliseconds trigger_delay(seed_epoch_millis - clock_epoch * 1000);

            VideoOverlaySet overlays(env.console.overlay());
            date_reader.make_overlays(overlays);
            date_reader.set_date(env.program_info(), env.console, context, set_date);

            //  Commit the date and start the timer.
            pbf_press_button(context, BUTTON_A, 20, 30);
            WallClock trigger_time = std::chrono::system_clock::now() + trigger_delay;
            env.log("Will commit in " + tostr_u_commas(trigger_delay.count()) + " milliseconds.");

            //  Re-enter the game.
            pbf_press_button(context, BUTTON_HOME, 20, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);
            resume_game_from_home(env.console, context, false);

            if (!prompt.detect(env.console.video().snapshot())){
                env.log("Expected to be on prompt menu. Backing out.", COLOR_RED);
                stats.errors++;
                env.update_stats();
                pbf_mash_button(context, BUTTON_B, 500);
                continue;
            }

            //  Wait for trigger time.
            context.wait_until(trigger_time);
            pbf_press_button(context, BUTTON_A, 10, 10);
            continue;
        }
        case 4:{
            env.log("Detected material selection.");
            if (printed){
                return;
            }
            if (!overworld_seen){
                pbf_press_button(context, BUTTON_B, 20, 30);
                continue;
            }
            item_printer_start_print(env.console, context, LANGUAGE, jobs);
            stats.prints++;
            env.update_stats();
            printed = true;
            item_printer_finish_print(env.inference_dispatcher(), env.console, context, LANGUAGE);
//            pbf_press_button(context, BUTTON_B, 20, 30);
            continue;
        }
        default:
            stats.errors++;
            env.update_stats();
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT,
                env.logger(),
                ""
            );
        }
    }
}
void ItemPrinterRNG::print_again(
    SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    ItemPrinterJobs jobs
) const{
    ItemPrinterRNG_Descriptor::Stats& stats = env.current_stats<ItemPrinterRNG_Descriptor::Stats>();

    bool printed = false;
    while (true){
        context.wait_for_all_requests();

        OverworldWatcher overworld(COLOR_BLUE);
        AdvanceDialogWatcher dialog(COLOR_RED);
//        PromptDialogWatcher prompt(COLOR_GREEN);
        WhiteButtonWatcher material(COLOR_GREEN, WhiteButton::ButtonX, {0.63, 0.93, 0.17, 0.06});
        int ret = wait_until(
            env.console, context, std::chrono::seconds(120),
            {
                overworld,
                dialog,
//                prompt,
                material,
            }
        );
        switch (ret){
        case 0:
            env.log("Detected overworld... (unexpected)", COLOR_RED);
            return;

        case 1:
            env.log("Detected advance dialog.");
            pbf_press_button(context, BUTTON_B, 20, 30);
            continue;

        case 2:{
            env.log("Detected material selection.");
            if (printed){
                return;
            }
            item_printer_start_print(env.console, context, LANGUAGE, jobs);
            stats.prints++;
            env.update_stats();
            printed = true;
            item_printer_finish_print(env.inference_dispatcher(), env.console, context, LANGUAGE);
            continue;
        }
        default:
            stats.errors++;
            env.update_stats();
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT,
                env.logger(),
                ""
            );
        }
    }
}


void ItemPrinterRNG::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);
    ItemPrinterRNG_Descriptor::Stats& stats = env.current_stats<ItemPrinterRNG_Descriptor::Stats>();

    env.update_stats();
    for (uint32_t c = 0; c < TOTAL_ROUNDS; c++){
        std::vector<ItemPrinterRngRowSnapshot> table = TABLE.snapshot<ItemPrinterRngRowSnapshot>();
        for (const ItemPrinterRngRowSnapshot& row : table){
            if (row.chain){
                print_again(env, context, row.jobs);
            }else{
                run_print_at_date(env, context, row.date, row.jobs);
            }
        }
//        run_print_at_date(env, context, DATE0, 1);
//        run_print_at_date(env, context, DATE1, 10);
        stats.iterations++;
        env.update_stats();
    }

    if (FIX_TIME_WHEN_DONE){
        pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY);
        home_to_date_time(context, false, false);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_HOME, 20, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);
        resume_game_from_home(env.console, context);
    }
    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}




























}
}
}
