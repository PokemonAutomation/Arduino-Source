/*  Item Printer RNG
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Qt/TimeQt.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/ItemPrinter/PokemonSV_ItemPrinterMenuDetector.h"
#include "PokemonSV/Inference/ItemPrinter/PokemonSV_ItemPrinterMaterialDetector.h"
#include "PokemonSV/Programs/Farming/PokemonSV_MaterialFarmerTools.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV_ItemPrinterSeedCalc.h"
#include "PokemonSV_ItemPrinterDatabase.h"
#include "PokemonSV_ItemPrinterRNG.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



ItemPrinterRNG_Descriptor::ItemPrinterRNG_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:ItemPrinterRNG",
        Pokemon::STRING_POKEMON + " SV", "Item Printer RNG",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/ItemPrinterRNG.md",
        "Farm the Item Printer using RNG Manipulation.",
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct ItemPrinterRNG_Descriptor::Stats : public StatsTracker{
    Stats()
        : iterations(m_stats["Rounds"])
        , prints(m_stats["Prints"])
//        , total_jobs(m_stats["Total Jobs"])
        , frame_hits(m_stats["Frame Hits"])
        , frame_misses(m_stats["Frame Misses"])
        , frame_unknown(m_stats["Unknown Frame"])
        , material_farmer_runs(m_stats["Material Farmer Runs"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Rounds");
        m_display_order.emplace_back("Prints");
//        m_display_order.emplace_back("Total Jobs", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Frame Hits");
        m_display_order.emplace_back("Frame Misses");
        m_display_order.emplace_back("Unknown Frame", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Material Farmer Runs", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& iterations;
    std::atomic<uint64_t>& prints;
//    std::atomic<uint64_t>& total_jobs;
    std::atomic<uint64_t>& frame_hits;
    std::atomic<uint64_t>& frame_misses;
    std::atomic<uint64_t>& frame_unknown;
    std::atomic<uint64_t>& material_farmer_runs;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> ItemPrinterRNG_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

ItemPrinterRNG::~ItemPrinterRNG(){
    MATERIAL_FARMER_OPTIONS.remove_listener(*this);
    MATERIAL_FARMER_TRIGGER.remove_listener(*this);
    DATE_SEED_TABLE.remove_listener(*this);
//    AUTO_MATERIAL_FARMING.remove_listener(*this);
}

ItemPrinterRNG::ItemPrinterRNG()
    : LANGUAGE(
        "<b>Game Language:</b>",
        PokemonSwSh::IV_READER().languages(),
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , NUM_ITEM_PRINTER_ROUNDS(
        "<b>Number of rounds per Item Printer session:</b><br>Iterate the rounds table this many times. ",
        LockMode::UNLOCK_WHILE_RUNNING, 10
    )
//    , AFTER_ITEM_PRINTER_DONE_EXPLANATION(
//        "Then proceed to material farming."
//    )
    , OVERLAPPING_BONUS_WARNING(
        "<font color=\"red\">WARNING: At least one of the Ball/Item bonuses haven't been fully used up. "
        "This can mess up your rewards for subsequent prints.</font><br>"
        "<font color=\"red\">\nNote: Each Ball/Item bonus lasts for 10 prints.</font>"
    )
    , MODE(
        "<b>Item Printer mode:</b><br>",
        {
            {ItemPrinterMode::STANDARD_MODE, "standard", "Standard Mode: Manually select exactly what is being printed for each print job."},
            {ItemPrinterMode::AUTO_MODE, "auto", "Auto Mode: Select your desired item and its quantity, and items will be automatically printed."},
        },
        LockMode::LOCK_WHILE_RUNNING,
        ItemPrinterMode::STANDARD_MODE
    )
    , DESIRED_ITEM_TABLE(
        "<b>Item Table:</b><br>"
        "Input your desired item and desired quantity to the table.<br>"
        "Ensure you have enough BP and are maxed out on the following sandwich ingredients: Chorizo, Bananas, Mayonnaise, Whipped Cream. "
        "Also, ensure you have a strong lead pokemon for auto-battling, for farming materials.<br>"
        "If there are duplicate items in the table, only the higher quantity will be considered."
    )
    , DATE_SEED_TABLE(
        "<b>Rounds Table:</b><br>Run the following prints in order and repeat. "
        "Changes to this table take effect the next time the table starts from the beginning."
    )
    , DELAY_MILLIS(
        "<b>Delay (Milliseconds):</b><br>"
        "The delay from when you press A to when the game reads the date for the seed. "
        "For OLED Switches, this delay is about 500 milliseconds. For older Switches, this delay is closer to 1500 milliseconds. "
        "If the following option is enabled, this delay will be automatically adjusted based on how you miss the frames.",
        LockMode::UNLOCK_WHILE_RUNNING, 1000,
        0, 5000
    )
    , ADJUST_DELAY(
        "<b>Automatically Adjust Delay:</b><br>Adjust the delay, depending on the desired item and the actual print result.",
        LockMode::UNLOCK_WHILE_RUNNING, true
    )
    , GO_HOME_WHEN_DONE(false)
    , FIX_TIME_WHEN_DONE(
        "<b>Fix Time when Done:</b><br>Fix the time after the program finishes.",
        LockMode::UNLOCK_WHILE_RUNNING, true
    )
    , MATERIAL_FARMER_TRIGGER(
        "<b>Trigger to start material farmer:</b><br>",
        {
            {MaterialFarmerTrigger::FIXED_NUM_PRINT_JOBS, "fixed", "Start Material Farmer when done a certain number of print jobs."},
            {MaterialFarmerTrigger::MINIMUM_HAPPINY_DUST, "happiny-dust", "Start Material Farmer when Happiny Dust is less than a certain number."},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        MaterialFarmerTrigger::FIXED_NUM_PRINT_JOBS
    )
    , MATERIAL_FARMER_FIXED_NUM_JOBS(
        "<b>Print Jobs per Material Farming Session:</b><br>"
        "Run the material farmer once for this many jobs printed.",
        LockMode::UNLOCK_WHILE_RUNNING, 500,
        20, 650
    )
    , MIN_HAPPINY_DUST(
        "<b>Minimum Happiny Dust:</b><br>"
        "Run the material farmer before the number of Happiny Dust drops "
        "below this number.<br>"
        "This ensures no other material drops below this number. "
        "If a material starts below this threshold, it remains there.<br>"
        "Changes to this number only take place after returning to "
        "the item printer, after material farming.",
        LockMode::UNLOCK_WHILE_RUNNING, 400,
        1, 999
    )
    , MATERIAL_FARMER_OPTIONS(
        GroupOption::EnableMode::DEFAULT_DISABLED,
        &LANGUAGE,
        NOTIFICATION_STATUS_UPDATE,
        NOTIFICATION_PROGRAM_FINISH,
        NOTIFICATION_ERROR_RECOVERABLE,
        NOTIFICATION_ERROR_FATAL
    )
    , ENABLE_SEED_CALC(
        "<b>Enable Seed Calculation: (developer only)</b><br>"
        "Use Kurt's seed calculation instead of hardcoded database. Ensure you have the appropriate resources.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(LANGUAGE);

    PA_ADD_OPTION(MODE);
    PA_ADD_OPTION(DESIRED_ITEM_TABLE);
    PA_ADD_OPTION(NUM_ITEM_PRINTER_ROUNDS);
    PA_ADD_OPTION(DATE_SEED_TABLE);
    PA_ADD_OPTION(OVERLAPPING_BONUS_WARNING);
    PA_ADD_OPTION(DELAY_MILLIS);
    PA_ADD_OPTION(ADJUST_DELAY);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(FIX_TIME_WHEN_DONE);

    PA_ADD_OPTION(MATERIAL_FARMER_TRIGGER);
    PA_ADD_OPTION(MATERIAL_FARMER_FIXED_NUM_JOBS);
    PA_ADD_OPTION(MIN_HAPPINY_DUST);
    PA_ADD_OPTION(MATERIAL_FARMER_OPTIONS);
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(ENABLE_SEED_CALC);
    }

    PA_ADD_OPTION(NOTIFICATIONS);

    ItemPrinterRNG::on_config_value_changed(this);
//    AUTO_MATERIAL_FARMING.add_listener(*this);
    DATE_SEED_TABLE.add_listener(*this);
    MODE.add_listener(*this);
    MATERIAL_FARMER_OPTIONS.add_listener(*this);
    MATERIAL_FARMER_TRIGGER.add_listener(*this);
}

void ItemPrinterRNG::on_config_value_changed(void* object){

    NUM_ITEM_PRINTER_ROUNDS.set_visibility(
        MODE == ItemPrinterMode::AUTO_MODE ? ConfigOptionState::HIDDEN : ConfigOptionState::ENABLED
    );

    OVERLAPPING_BONUS_WARNING.set_visibility(
        (MODE != ItemPrinterMode::AUTO_MODE) && overlapping_bonus()
        ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN
    );

    DESIRED_ITEM_TABLE.set_visibility(
        MODE == ItemPrinterMode::AUTO_MODE ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN
    );

    DATE_SEED_TABLE.set_visibility(
        MODE == ItemPrinterMode::AUTO_MODE ? ConfigOptionState::HIDDEN : ConfigOptionState::ENABLED
    );

    ADJUST_DELAY.set_visibility(
        MODE == ItemPrinterMode::AUTO_MODE ? ConfigOptionState::HIDDEN : ConfigOptionState::ENABLED
    );

    MATERIAL_FARMER_TRIGGER.set_visibility(
        (MODE != ItemPrinterMode::AUTO_MODE) && MATERIAL_FARMER_OPTIONS.enabled()
        ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN
    );

    MATERIAL_FARMER_FIXED_NUM_JOBS.set_visibility(
        (MODE != ItemPrinterMode::AUTO_MODE) && MATERIAL_FARMER_OPTIONS.enabled() && (MATERIAL_FARMER_TRIGGER == MaterialFarmerTrigger::FIXED_NUM_PRINT_JOBS)
        ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN
    );    

    MIN_HAPPINY_DUST.set_visibility(
        (MODE != ItemPrinterMode::AUTO_MODE) && MATERIAL_FARMER_OPTIONS.enabled() && (MATERIAL_FARMER_TRIGGER == MaterialFarmerTrigger::MINIMUM_HAPPINY_DUST)
        ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN
    );

    MATERIAL_FARMER_OPTIONS.set_visibility(
        MODE == ItemPrinterMode::AUTO_MODE ? ConfigOptionState::HIDDEN : ConfigOptionState::ENABLED
    );

    ENABLE_SEED_CALC.set_visibility(
        MODE == ItemPrinterMode::AUTO_MODE ? ConfigOptionState::HIDDEN : ConfigOptionState::ENABLED
    );

}


// - return true if the user is trying to trigger another Ball/Item bonus without
// using up the previous bonus. Doing this can mess up rewards for subsequent prints, 
// since a ball bonus interferes with activating an item bonus and vice versa.
// - each Bonus lasts for 10 prints.
bool ItemPrinterRNG::overlapping_bonus(){
    // for each row in table. if ball/item bonus, ensure that sum of prints in subsequent rows >=10 before the next bonus, or end of the table.
    uint16_t total_prints_since_last_bonus = 10;
    for (std::shared_ptr<EditableTableRow>& table_row : DATE_SEED_TABLE.current_refs()){
        ItemPrinterRngRow& row = static_cast<ItemPrinterRngRow&>(*table_row);
        if (row.desired_item == ItemPrinter::PrebuiltOptions::BALL_BONUS || row.desired_item == ItemPrinter::PrebuiltOptions::ITEM_BONUS){
            if (total_prints_since_last_bonus < 10){
                return true;
            }
            total_prints_since_last_bonus = 0;
        }else{
            total_prints_since_last_bonus += (uint16_t)static_cast<ItemPrinterJobs>(row.jobs);
        }
    }

    if (total_prints_since_last_bonus < 10){
        return true;
    }

    return false;
}


ItemPrinterPrizeResult ItemPrinterRNG::run_print_at_date(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    const DateTime& date, ItemPrinterJobs jobs
){
    ItemPrinterRNG_Descriptor::Stats& stats = env.current_stats<ItemPrinterRNG_Descriptor::Stats>();
    ItemPrinterPrizeResult prize_result;
    bool printed = false;
    bool overworld_seen = false;
    size_t failures = 0;
    std::chrono::seconds next_wait_time = std::chrono::seconds(120);
    while (true){
        if (failures >= 5){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Unable to print after 5 attempts.",
                env.console
            );
        }
        context.wait_for_all_requests();

        OverworldWatcher overworld(env.console, COLOR_BLUE);
        AdvanceDialogWatcher dialog(COLOR_RED);
        PromptDialogWatcher prompt(COLOR_GREEN);
        DateChangeWatcher date_reader(env.console);
        WhiteButtonWatcher material(COLOR_GREEN, WhiteButton::ButtonX, {0.63, 0.93, 0.17, 0.06});
        int ret = wait_until(
            env.console, context, next_wait_time,
            {
                overworld,
                dialog,
                prompt,
                date_reader,
                material,
            }
        );
        context.wait_for(std::chrono::milliseconds(250));
        next_wait_time = std::chrono::seconds(120);
        switch (ret){
        case 0:
            overworld_seen = true;
            if (printed){
                env.log("Detected overworld... (unexpected)", COLOR_RED);
                return prize_result;
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
            go_home(env.console, context);
            home_to_date_time(env.console, context, true);
            pbf_press_button(context, BUTTON_A, 10, 30);
            context.wait_for_all_requests();
            next_wait_time = std::chrono::seconds(5);
            continue;
        }
        case 3:{
            env.log("Detected date change.");
            uint16_t delay_mills = DELAY_MILLIS;

            //  This is the seed we intend to hit.
            uint64_t seed_epoch_millis = to_seconds_since_epoch(date) * 1000;
            seed_epoch_millis -= delay_mills;

            //  This is the time we intend to set the clock to.
            uint64_t threshold = delay_mills + 3000;
            uint64_t clock_epoch = (seed_epoch_millis - threshold) / 1000;
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
            pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
            resume_game_from_home(env.console, context, false);

            context.wait_for(250ms);

            VideoSnapshot snapshot = env.console.video().snapshot();
            if (!prompt.detect(snapshot)){
                env.log("Expected to be on prompt menu. Backing out.", COLOR_RED);
//                snapshot->save("noprompt.png");
                stats.errors++;
                env.update_stats();
                pbf_mash_button(context, BUTTON_B, 500);
                continue;
            }

            //  Wait for trigger time.
            context.wait_until(trigger_time);
            pbf_press_button(context, BUTTON_A, 80ms, 500ms);
            continue;
        }
        case 4:{
            env.log("Detected material selection.");
            if (printed){
                return prize_result;
            }
            if (!overworld_seen){
                pbf_press_button(context, BUTTON_B, 20, 30);
                continue;
            }
            item_printer_start_print(env.console, context, LANGUAGE, jobs);
            stats.prints++;
            env.update_stats();
            printed = true;
            prize_result = item_printer_finish_print(env.console, context, LANGUAGE);
            std::array<std::string, 10> print_results = prize_result.prizes;
            uint64_t seed = to_seconds_since_epoch(date);
            int distance_from_target = get_distance_from_target(env.console, stats, print_results, seed);
            env.update_stats();
            if ((ADJUST_DELAY || MODE == ItemPrinterMode::AUTO_MODE) &&
                distance_from_target != 0 &&
                distance_from_target != std::numeric_limits<int>::min()
            ){
                adjust_delay(env.logger(), env, print_results, distance_from_target);
            }

//            pbf_press_button(context, BUTTON_B, 20, 30);
            continue;
        }
        default:
            failures++;
            stats.errors++;
            env.update_stats();
            env.console.log("No state detected after 2 minutes.", COLOR_RED);
#if 0
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "No state detected after 2 minutes.",
                env.console
            );
#endif
        }
    }
}

void ItemPrinterRNG::adjust_delay(
    Logger& logger,
    SingleSwitchProgramEnvironment& env,
    const std::array<std::string, 10>& print_results,
    int distance_from_target
){
    const int MAX_MISS = 2;
    const int16_t ADJUSTMENT_TABLE[MAX_MISS + 1] = {0, 50, 1000};

    int16_t delay_adjustment;
    if (distance_from_target < 0){
        distance_from_target = std::min(-distance_from_target, MAX_MISS);
        delay_adjustment = -ADJUSTMENT_TABLE[distance_from_target];
    }else if (distance_from_target > 0){
        distance_from_target = std::min(distance_from_target, MAX_MISS);
        delay_adjustment = ADJUSTMENT_TABLE[distance_from_target];
    }else{
        return;
    }

    int16_t new_delay = DELAY_MILLIS + delay_adjustment;
    new_delay = std::max(new_delay, (int16_t)DELAY_MILLIS.min_value());
    new_delay = std::min(new_delay, (int16_t)DELAY_MILLIS.max_value());
    DELAY_MILLIS.set((uint16_t)new_delay);
    logger.log("Current delay: " + std::to_string(new_delay));
    // std::cout << "Current delay:" << new_delay << std::endl;

}

int ItemPrinterRNG::get_distance_from_target(
    Logger& logger,
    ItemPrinterRNG_Descriptor::Stats& stats,
    const std::array<std::string, 10>& print_results,
    uint64_t seed
){
    int distance_from_target = std::numeric_limits<int>::min();
    const int MAX_DEVIATION = 10;
    for (int current_deviation = 0; current_deviation <= MAX_DEVIATION; current_deviation++){
        ItemPrinter::DateSeed seed_data;
        if (ENABLE_SEED_CALC || MODE == ItemPrinterMode::AUTO_MODE){
            seed_data = ItemPrinter::calculate_seed_prizes(seed - current_deviation);
        }else{
            seed_data = ItemPrinter::get_date_seed(seed - current_deviation);
        }
        if (results_approximately_match(print_results, seed_data.regular) ||
            results_approximately_match(print_results, seed_data.item_bonus) ||
            results_approximately_match(print_results, seed_data.ball_bonus)
        ){
            distance_from_target = -current_deviation;
            break;
        }

        if (current_deviation == 0){
            continue;
        }

        if (ENABLE_SEED_CALC || MODE == ItemPrinterMode::AUTO_MODE){
            seed_data = ItemPrinter::calculate_seed_prizes(seed + current_deviation);
        }else{
            seed_data = ItemPrinter::get_date_seed(seed + current_deviation);
        }
        if (results_approximately_match(print_results, seed_data.regular) ||
            results_approximately_match(print_results, seed_data.item_bonus) ||
            results_approximately_match(print_results, seed_data.ball_bonus)
        ){
            distance_from_target = current_deviation;
            break;
        }
    }

    if (distance_from_target == std::numeric_limits<int>::min()){
        logger.log("Frame Result: Unable to determine frame.", COLOR_RED);
        stats.frame_unknown++;
    }else if (distance_from_target < 0){
        logger.log("Frame Result: Missed. (Target - " + std::to_string(-distance_from_target) + ")", COLOR_ORANGE);
        stats.frame_misses++;
    }else if (distance_from_target > 0){
        logger.log("Frame Result: Missed. (Target + " + std::to_string(distance_from_target) + ")", COLOR_ORANGE);
        stats.frame_misses++;
    }else{
        logger.log("Frame Result: Target hit", COLOR_BLUE);
        stats.frame_hits++;
    }

    return distance_from_target;
}



bool ItemPrinterRNG::results_approximately_match(
    const std::array<std::string, 10>& print_results, 
    const std::array<std::string, 10>& expected_result
){
    size_t total_items = 0;
    size_t mismatches = 0;
    for (uint8_t i = 0; i < 10; i++){
        const std::string& x = print_results[i];
        const std::string& y = expected_result[i];
        if (x.empty() || y.empty()){
            continue;
        }

        total_items++;

        if (x == y){
            continue;
        }

        mismatches++;
    }

    if (total_items == 0){
        return false;
    }

    return mismatches * 5 <= total_items;
}

void ItemPrinterRNG::print_again(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    ItemPrinterJobs jobs
) const{
    ItemPrinterRNG_Descriptor::Stats& stats = env.current_stats<ItemPrinterRNG_Descriptor::Stats>();

    bool printed = false;
    while (true){
        context.wait_for_all_requests();

        OverworldWatcher overworld(env.console, COLOR_BLUE);
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
            item_printer_finish_print(env.console, context, LANGUAGE);
            continue;
        }
        default:
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "No state detected after 2 minutes.",
                env.console
            );
        }
    }
}

void ItemPrinterRNG::run_item_printer_rng_automode(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    ItemPrinterRNG_Descriptor::Stats& stats
){
    const uint16_t min_happiny_dust = 400;

    MaterialFarmerOptions material_farmer_options(
        GroupOption::EnableMode::DEFAULT_ENABLED,
        &LANGUAGE,
        NOTIFICATION_STATUS_UPDATE,
        NOTIFICATION_PROGRAM_FINISH,
        NOTIFICATION_ERROR_RECOVERABLE,
        NOTIFICATION_ERROR_FATAL
    );

    material_farmer_options.RUN_TIME_IN_MINUTES.set(32);
    material_farmer_options.SANDWICH_OPTIONS.set_enabled(true);
    material_farmer_options.SANDWICH_OPTIONS.SAVE_GAME_BEFORE_SANDWICH = true;
    material_farmer_options.SANDWICH_OPTIONS.BASE_RECIPE.set(BaseRecipe::non_shiny);

    //  For each job that we print, we increment jobs_counter.
    //  Each time we run the material farmer, we reset jobs_counter to 0.
    uint32_t jobs_counter = 0;

    // Check material quantity when:
    // - once when first starting the item printer
    // - before starting material farming. If still have material, 
    // can keep using item printer. But this check is only done once, 
    // until you farm materials again.
    // - when back from material farming
    bool done_one_last_material_check_before_mat_farming = false;
    uint32_t material_farmer_jobs_period = calc_num_jobs_using_happiny_dust(env, context, min_happiny_dust);   
    bool have_cleared_out_bonus = false;
    std::map<std::string, uint16_t> obtained_prizes;

    std::vector<ItemPrinterDesiredItemRowSnapshot> desired_table = DESIRED_ITEM_TABLE.snapshot<ItemPrinterDesiredItemRowSnapshot>();
    for (const ItemPrinterDesiredItemRowSnapshot& desired_row : desired_table){
        std::string desired_slug = ItemPrinter::PrebuiltOptions_AutoMode_Database().find(desired_row.item)->slug;
        int16_t desired_quantity = desired_row.quantity;
        int16_t obtained_quantity = check_obtained_quantity(obtained_prizes, desired_slug);
        
        while (obtained_quantity < desired_quantity){
            int16_t quantity_to_print = desired_quantity - obtained_quantity;
            std::vector<ItemPrinterRngRowSnapshot> print_table = desired_print_table(desired_row.item, quantity_to_print);
            if (!have_cleared_out_bonus){
                // 2323229535, 8 Ability Patches, with no bonus active
                // x2 Magnet, x9 Exp. Candy S, x7 Pretty Feather, x2 Ability Patch, x2 Ability Patch, 
                // x7 Big Pearl, x1 Ability Patch, x1 Ability Patch, x16 Ground Tera Shard, x2 Ability Patch     

                // start with printing out 10 just to clear out any lingering bonuses.       
                ItemPrinterRngRowSnapshot print_10_items = {false, from_seconds_since_epoch(2323229535), ItemPrinterJobs::Jobs_10};
                print_table.insert(print_table.begin(), print_10_items);
                have_cleared_out_bonus = true;
            }
            
            for (const ItemPrinterRngRowSnapshot& row : print_table){
                env.console.log(desired_slug + ": " + std::to_string(check_obtained_quantity(obtained_prizes, desired_slug)) + "/" + std::to_string(desired_quantity));

                // check if need to run material farmer
                while (jobs_counter >= material_farmer_jobs_period){ 
                    if (!done_one_last_material_check_before_mat_farming){
                        // one more material quantity check before material farming
                        // if still have material, keep using item printer
                        material_farmer_jobs_period = calc_num_jobs_using_happiny_dust(env, context, min_happiny_dust);
                        jobs_counter = 0;
                        done_one_last_material_check_before_mat_farming = true;
                        continue;
                    }

                    //  Run the material farmer.
                    run_material_farming_then_return_to_item_printer(env, context, stats, material_farmer_options);

                    // Recheck number of Happiny Dust after returning from Material Farming,
                    // prior to restarting Item printing
                    material_farmer_jobs_period = calc_num_jobs_using_happiny_dust(env, context, min_happiny_dust);
                    jobs_counter = 0;
                    done_one_last_material_check_before_mat_farming = false;
                }

                ItemPrinterPrizeResult prize_result = run_print_at_date(env, context, row.date, row.jobs);
                std::array<std::string, 10> prizes = prize_result.prizes;
                std::array<int16_t, 10> quantities = prize_result.quantities;
                for (size_t i = 0; i < 10; i++){
                    obtained_prizes[prizes[i]] += quantities[i];
                }

                jobs_counter += (uint32_t)row.jobs;
                env.console.log("Print job counter: " + std::to_string(jobs_counter) + "/" + std::to_string(material_farmer_jobs_period));
                env.console.log("Cumulative prize list:");
                for (const auto& prize : obtained_prizes){
                    if (prize.first == ""){
                        continue;
                    }
                    env.console.log(prize.first + ": " + std::to_string(prize.second));
                }                   
                
            }

            obtained_quantity = check_obtained_quantity(obtained_prizes, desired_slug);
        }

        
        // stats.iterations++;
        // env.update_stats();
    }
  
}

int16_t ItemPrinterRNG::check_obtained_quantity(std::map<std::string, uint16_t> obtained_prizes, std::string desired_slug){
    int16_t obtained_quantity = 0;
    auto prize_iter = obtained_prizes.find(desired_slug);
    if (prize_iter != obtained_prizes.end()){
        obtained_quantity = prize_iter->second;
    }

    return obtained_quantity;
}


void ItemPrinterRNG::run_material_farming_then_return_to_item_printer(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context, 
    ItemPrinterRNG_Descriptor::Stats& stats,
    MaterialFarmerOptions& material_farmer_options
){
    press_Bs_to_back_to_overworld(env.program_info(), env.console, context);
    move_from_item_printer_to_material_farming(env.program_info(), env.console, context);
    {
        //  Dummy stats since we don't use the material farmer stats.
        MaterialFarmerStats mat_farm_stats;
        run_material_farmer(env, env.console, context, material_farmer_options, mat_farm_stats);
        stats.material_farmer_runs++;
        env.update_stats();
    }
    move_from_material_farming_to_item_printer(env.program_info(), env.console, context);

}

std::vector<ItemPrinterRngRowSnapshot> ItemPrinterRNG::desired_print_table(
    ItemPrinter::PrebuiltOptions desired_item,
    uint16_t quantity_to_print
){
    ItemPrinter::ItemPrinterEnumOption desired_enum_option = option_lookup_by_enum(desired_item);

    // one bonus bundle is Item/Ball Bonus -> 5 print -> 5 print
    // quantity_obtained stores the quantity of the desired item that
    // is produced with one 5 print, with the bonus active.
    uint16_t num_bonus_bundles = (quantity_to_print + (desired_enum_option.quantity_obtained * 2) - 1)/(desired_enum_option.quantity_obtained * 2); // round up after dividing

    ItemPrinter::PrebuiltOptions bonus_type = get_bonus_type(desired_item);
    ItemPrinter::ItemPrinterEnumOption bonus_enum_option = option_lookup_by_enum(bonus_type);
    ItemPrinterRngRowSnapshot bonus_snapshot = {false, from_seconds_since_epoch(bonus_enum_option.seed), bonus_enum_option.jobs};    
    ItemPrinterRngRowSnapshot desired_item_snapshot = {false, from_seconds_since_epoch(desired_enum_option.seed), desired_enum_option.jobs};

    std::vector<ItemPrinterRngRowSnapshot> print_table;
    for (size_t i = 0; i < num_bonus_bundles; i++){
        print_table.push_back(bonus_snapshot);
        // - we assume all the desired item prints are 5 prints,
        // since all the single item prints stored in the database are 5 prints.
        print_table.push_back(desired_item_snapshot); 
        print_table.push_back(desired_item_snapshot);
    }

    // cout << "Total number of bonus bundles: " << num_bonus_bundles << endl;
    // for (const ItemPrinterRngRowSnapshot& row : print_table){
    //     cout << (int)row.jobs << ": ";
    //     std::cout << row.date.year << "-" << (int)row.date.month << "-" << (int)row.date.day << " ";
    //     std::cout << (int)row.date.hour << ":" << (int)row.date.minute << ":" << (int)row.date.second << endl;
    // }

    return print_table;
}


ItemPrinter::PrebuiltOptions ItemPrinterRNG::get_bonus_type(ItemPrinter::PrebuiltOptions desired_item){
    // EnumDropdownDatabase<ItemPrinter::PrebuiltOptions> database = ItemPrinter::PrebuiltOptions_AutoMode_Database();
    // std::string slug = database.find(desired_item)->slug;
    std::string slug = ItemPrinter::PrebuiltOptions_AutoMode_Database().find(desired_item)->slug;
    // cout << slug << endl;
    if (slug.find("ball") != std::string::npos){
        return ItemPrinter::PrebuiltOptions::BALL_BONUS;
    }else{
        return ItemPrinter::PrebuiltOptions::ITEM_BONUS;
    }
}


void ItemPrinterRNG::run_item_printer_rng(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context, 
    ItemPrinterRNG_Descriptor::Stats& stats
){
    //  For each job that we print, we increment jobs_counter.
    //  Each time we run the material farmer, we reset jobs_counter to 0.
    uint32_t jobs_counter = 0;

    bool done_one_last_material_check_before_mat_farming = false;
    uint32_t material_farmer_jobs_period = MATERIAL_FARMER_FIXED_NUM_JOBS;
    if (MATERIAL_FARMER_OPTIONS.enabled() && MATERIAL_FARMER_TRIGGER == MaterialFarmerTrigger::MINIMUM_HAPPINY_DUST){
        // Check material quantity when:
        // - once when first starting the item printer
        // - before starting material farming. If still have material, 
        // can keep using item printer. But this check is only done once, 
        // until you farm materials again.
        // - when back from material farming

        uint32_t num_jobs_with_happiny_dust = calc_num_jobs_using_happiny_dust(env, context, MIN_HAPPINY_DUST);
        material_farmer_jobs_period = num_jobs_with_happiny_dust;
    }

    for (uint32_t c = 0; c < NUM_ITEM_PRINTER_ROUNDS; c++){
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        std::vector<ItemPrinterRngRowSnapshot> table = DATE_SEED_TABLE.snapshot<ItemPrinterRngRowSnapshot>();
        for (const ItemPrinterRngRowSnapshot& row : table){
            //  Cannot run material farmer between chained prints.
            if (row.chain){
                print_again(env, context, row.jobs);
                jobs_counter += (uint32_t)row.jobs;
                continue;
            }

            run_print_at_date(env, context, row.date, row.jobs);
            jobs_counter += (uint32_t)row.jobs;

            if (!MATERIAL_FARMER_OPTIONS.enabled()){
                continue;
            }

            ////////////////////////////////////////////////////
            //  Material farmer is enabled. 
            //  Check number of print jobs before triggering material farmer.
            ////////////////////////////////////////////////////

            if (MATERIAL_FARMER_TRIGGER == MaterialFarmerTrigger::FIXED_NUM_PRINT_JOBS){
                material_farmer_jobs_period = MATERIAL_FARMER_FIXED_NUM_JOBS;
            }
            env.console.log(
                "Print job counter: " 
                + std::to_string(jobs_counter) 
                + "/" 
                + std::to_string(material_farmer_jobs_period)
            );
            
            //  Not ready to run material farmer yet.
            if (jobs_counter < material_farmer_jobs_period){
                continue;
            }
            
            if (MATERIAL_FARMER_TRIGGER == MaterialFarmerTrigger::MINIMUM_HAPPINY_DUST 
                && !done_one_last_material_check_before_mat_farming
            ){
                // one more material quantity check before material farming
                // if still have material, keep using item printer
                uint32_t num_jobs_with_happiny_dust = calc_num_jobs_using_happiny_dust(env, context, MIN_HAPPINY_DUST);
                material_farmer_jobs_period = num_jobs_with_happiny_dust;
                jobs_counter = 0;
                done_one_last_material_check_before_mat_farming = true;
                if (material_farmer_jobs_period > 0){
                    continue;
                }                
            }            

            //  Run the material farmer.
            press_Bs_to_back_to_overworld(env.program_info(), env.console, context);
            move_from_item_printer_to_material_farming(env.program_info(), env.console, context);
            {
                //  Dummy stats since we don't use the material farmer stats.
                MaterialFarmerStats mat_farm_stats;
                run_material_farmer(env, env.console, context, MATERIAL_FARMER_OPTIONS, mat_farm_stats);
                stats.material_farmer_runs++;
                env.update_stats();
            }
            move_from_material_farming_to_item_printer(env.program_info(), env.console, context);

            // Recheck number of Happiny Dust after returning from Material Farming,
            // prior to restarting Item printing
            if (MATERIAL_FARMER_TRIGGER == MaterialFarmerTrigger::MINIMUM_HAPPINY_DUST){
                uint32_t num_jobs_with_happiny_dust = calc_num_jobs_using_happiny_dust(env, context, MIN_HAPPINY_DUST);
                material_farmer_jobs_period = num_jobs_with_happiny_dust;
                done_one_last_material_check_before_mat_farming = false;
            }
            
            jobs_counter = 0;
        }
//        run_print_at_date(env, context, DATE0, 1);
//        run_print_at_date(env, context, DATE1, 10);
        stats.iterations++;
        env.update_stats();
    }
}

// return number of print jobs we can do, based on how much Happiny Dust we have,
// and how low we allow the Happiny dust to go (min_happiny_dust)
uint32_t ItemPrinterRNG::calc_num_jobs_using_happiny_dust(
    SingleSwitchProgramEnvironment& env, 
    ProControllerContext& context,
    uint16_t min_happiny_dust
){
    uint32_t num_happiny_dust = check_num_happiny_dust(env, context);

    // give a buffer of 50, for a margin of safety. signed int to handle negative numbers
    int32_t num_happiny_dust_can_use = num_happiny_dust - MIN_HAPPINY_DUST - 50; 
    num_happiny_dust_can_use = num_happiny_dust_can_use < 0 ? 0 : num_happiny_dust_can_use;

    // assume 62% value for Happiny Dust to account for item printer wasteage.
    uint32_t num_print_jobs = (uint32_t)(num_happiny_dust_can_use * 0.62); // truncate the float back to int
    env.console.log("Number of Happiny Dust we have: " + std::to_string(num_happiny_dust));
    env.console.log("Number of Happiny Dust we can use (with some safety margins): " + std::to_string(num_happiny_dust_can_use));
    env.console.log("Number of print jobs we can do before material farming: " + std::to_string(num_print_jobs));
    return num_print_jobs;
}

uint32_t ItemPrinterRNG::check_num_happiny_dust(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context
){
    ItemPrinterRNG_Descriptor::Stats& stats = env.current_stats<ItemPrinterRNG_Descriptor::Stats>();
    env.log("Check how much Happiny Dust we have.");
    uint32_t num_happiny_dust;
    while (true){
        context.wait_for_all_requests();

        OverworldWatcher overworld(env.console, COLOR_BLUE);
        AdvanceDialogWatcher dialog(COLOR_RED);
        PromptDialogWatcher prompt(COLOR_GREEN);
        DateChangeWatcher date_reader(env.console);
        ItemPrinterMenuWatcher material(COLOR_GREEN);
        int ret = wait_until(
            env.console, context, std::chrono::seconds(120),
            {
                overworld,
                dialog,
                prompt,
                material,
            }
        );
        switch (ret){
        case 0:
            env.log("Detected overworld... Entering item printer.");
            pbf_press_button(context, BUTTON_A, 20, 30);
            continue;

        case 1:
            env.log("Detected advance dialog.");
            pbf_press_button(context, BUTTON_B, 20, 30);
            continue;

        case 2:{
            env.log("Detected prompt dialog. Entering item printer.");
            pbf_press_button(context, BUTTON_A, 10, 30);
            context.wait_for_all_requests();
            continue;
        }
        case 3:{
            env.log("Detected material selection.");
            ItemPrinterMaterialDetector detector(COLOR_RED, LANGUAGE);
            
            int8_t happiny_dust_row_num = detector.find_happiny_dust_row_index(
                env.console, context
            );

            context.wait_for_all_requests();
            VideoSnapshot snapshot = env.console.video().snapshot();
            num_happiny_dust = detector.detect_material_quantity(
                env.console, snapshot, context,
                happiny_dust_row_num
            );
            pbf_mash_button(context, BUTTON_B, 100);
            return num_happiny_dust;
        }
        default:
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "No state detected after 2 minutes.",
                env.console
            );
        }
    }    

    return 0;
}

void ItemPrinterRNG::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);
    ItemPrinterRNG_Descriptor::Stats& stats = env.current_stats<ItemPrinterRNG_Descriptor::Stats>();
    env.update_stats();


    if (MODE == ItemPrinterMode::AUTO_MODE || MATERIAL_FARMER_OPTIONS.enabled()){
        // - Ensure audio input is enabled
        LetsGoKillSoundDetector audio_detector(env.console, [](float){ return true; });
        wait_until(
            env.console, context,
            std::chrono::milliseconds(1100),
            {audio_detector}
        );
        audio_detector.throw_if_no_sound(std::chrono::milliseconds(1000));
    }

    try{
        if (MODE == ItemPrinterMode::AUTO_MODE){
            run_item_printer_rng_automode(env, context, stats);
        }else{
            run_item_printer_rng(env, context, stats);
        }

    }catch (ProgramFinishedException&){}

    if (FIX_TIME_WHEN_DONE){
        go_home(env.console, context);
        home_to_date_time(env.console, context, false);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
        resume_game_from_home(env.console, context);
    }
    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}




























}
}
}
