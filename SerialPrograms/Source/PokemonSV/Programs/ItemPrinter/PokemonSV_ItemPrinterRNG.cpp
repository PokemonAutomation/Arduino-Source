/*  Item Printer RNG
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Qt/TimeQt.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
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
#include "PokemonSV/Programs/Farming/PokemonSV_MaterialFarmerTools.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV_ItemPrinterRNG.h"
#include <iostream>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



ItemPrinterRngRow::~ItemPrinterRngRow(){
    chain.remove_listener(*this);
    date.remove_listener(*this);
//    jobs.remove_listener(*this);
    desired_item.remove_listener(*this);
}
ItemPrinterRngRow::ItemPrinterRngRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , chain(LockMode::UNLOCK_WHILE_RUNNING, false)
    , date(
        LockMode::UNLOCK_WHILE_RUNNING,
        DateTimeOption::DATE_HOUR_MIN_SEC,
        DateTime{2000, 1, 1, 0, 1, 0},
        DateTime{2060, 12, 31, 23, 59, 59},
        DateTime{2024,  6,  4,  0, 37,  8}
    )
    , jobs(
        ItemPrinterJobs_Database(),
        LockMode::UNLOCK_WHILE_RUNNING,
        ItemPrinterJobs::Jobs_1
    )
    , desired_item(
        ItemPrinterItems_Database(),
        LockMode::UNLOCK_WHILE_RUNNING,
        ItemPrinterItems::NONE
    )
//    , prev_desired_item(ItemPrinterItems::NONE)
{
    PA_ADD_OPTION(chain);
    PA_ADD_OPTION(date);
    PA_ADD_OPTION(jobs);
    PA_ADD_OPTION(desired_item);

    chain.add_listener(*this);
    date.add_listener(*this);
//    jobs.add_listener(*this);
    desired_item.add_listener(*this);

    ItemPrinterRngRow::value_changed(this);
}
ItemPrinterRngRow::ItemPrinterRngRow(
    EditableTableOption& parent_table,
    bool p_chain, const DateTime& p_date, ItemPrinterJobs p_jobs
)
    : ItemPrinterRngRow(parent_table)
{
    chain = p_chain;
    date.set(p_date);
    jobs.set(p_jobs);

    ItemPrinterRngRow::value_changed(this);
}
ItemPrinterRngRowSnapshot ItemPrinterRngRow::snapshot() const{
    return ItemPrinterRngRowSnapshot{chain, date, jobs};
}
std::unique_ptr<EditableTableRow> ItemPrinterRngRow::clone() const{
    std::unique_ptr<ItemPrinterRngRow> ret(new ItemPrinterRngRow(parent()));
    ret->chain = (bool)chain;
    ret->date.set(date);
    ret->jobs.set(jobs);
    return ret;
}



// - always update the date's visibility when chain is changed.
// - if desired_item has changed, set the seed (and number of jobs) accordingly
// - if any other value changes, set desired_item to NONE
void ItemPrinterRngRow::value_changed(void* object){
    date.set_visibility(chain ? ConfigOptionState::HIDDEN : ConfigOptionState::ENABLED);

    //  User initiated desired item change.
    if (object == &desired_item){
        ItemPrinterItems option = desired_item;
        if (option != ItemPrinterItems::NONE){
            const ItemPrinterEnumOption& option_data = option_lookup_by_enum(option);
            chain = false;
            date.set(from_seconds_since_epoch(option_data.seed));
            jobs.set(option_data.jobs);
        }
        return;
    }

    //  Other change.

    if (chain){
        desired_item.set(ItemPrinterItems::NONE);
        return;
    }

    //  See if the seed matches a known seed.
    const ItemPrinterEnumOption* option_data = option_lookup_by_seed(to_seconds_since_epoch(date));
    if (option_data == nullptr){
        desired_item.set(ItemPrinterItems::NONE);
    }else{
        desired_item.set(option_data->enum_value);
    }
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
        "Desired Item",
    };
}
std::vector<std::unique_ptr<EditableTableRow>> ItemPrinterRngTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(std::make_unique<ItemPrinterRngRow>(*this, false, DateTime{2024,  6,  4,  0, 37,  8}, ItemPrinterJobs::Jobs_1));
    ret.emplace_back(std::make_unique<ItemPrinterRngRow>(*this, false, DateTime{2049,  8, 18, 23, 51,  8}, ItemPrinterJobs::Jobs_10));
    ret.emplace_back(std::make_unique<ItemPrinterRngRow>(*this, false, DateTime{2024,  6,  4,  0, 37,  8}, ItemPrinterJobs::Jobs_1));
    ret.emplace_back(std::make_unique<ItemPrinterRngRow>(*this, false, DateTime{2031, 10,  8,  7,  9,  9}, ItemPrinterJobs::Jobs_10));
    ret.emplace_back(std::make_unique<ItemPrinterRngRow>(*this, false, DateTime{2024,  6,  4,  0, 37,  8}, ItemPrinterJobs::Jobs_1));
    ret.emplace_back(std::make_unique<ItemPrinterRngRow>(*this, false, DateTime{2020,  3,  3,  6, 38, 18}, ItemPrinterJobs::Jobs_10));
    return ret;
}




ItemPrinterRNG_Descriptor::ItemPrinterRNG_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:ItemPrinterRNG",
        Pokemon::STRING_POKEMON + " SV", "Item Printer RNG",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/ItemPrinterRNG.md",
        "Farm the Item Printer using RNG Manipulation.",
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

struct ItemPrinterRNG_Descriptor::Stats : public StatsTracker{
    Stats()
        : iterations(m_stats["Rounds"])
        , prints(m_stats["Prints"])
//        , total_jobs(m_stats["Total Jobs"])
        , frame_hits(m_stats["Frame Hits"])
        , frame_misses(m_stats["Frame Misses"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Rounds");
        m_display_order.emplace_back("Prints");
//        m_display_order.emplace_back("Total Jobs", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Frame Hits", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Frame Misses", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& iterations;
    std::atomic<uint64_t>& prints;
//    std::atomic<uint64_t>& total_jobs;
    std::atomic<uint64_t>& frame_hits;
    std::atomic<uint64_t>& frame_misses;
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
    , NUM_ITEM_PRINTER_ROUNDS(
        "<b>Number of rounds per Item Printer session:</b><br>Iterate the rounds table this many times. ",
        LockMode::UNLOCK_WHILE_RUNNING, 10
    )
    , AFTER_ITEM_PRINTER_DONE_EXPLANATION(
        "Then proceed to material farming."
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
        0, 5000
    )
    , ADJUST_DELAY(
        "<b>Automatically adjust delay:</b><br>Adjust the delay, depending on the desired item and the actual print result. "
        "This only works if the \"Desired Item\" column is used for selecting Date Seeds. ",
        LockMode::UNLOCK_WHILE_RUNNING, true
    )
    , GO_HOME_WHEN_DONE(false)
    , FIX_TIME_WHEN_DONE(
        "<b>Fix Time When Done:</b><br>Fix the time after the program finishes.",
        LockMode::UNLOCK_WHILE_RUNNING, true
    )
    , AUTO_MATERIAL_FARMING(
        "<b>Automatic Material Farming:</b><br>"
        "After using the item printer, automatically fly to North Province (Area 3) to farm materials, "
        "then fly back to keep using the item printer.",
        // "This will do the item printer loops, then the material farmer loops, then repeat.<br>",
        // "The number of item printer loops is set by \"Number of rounds per Item Printer session\".<br>"
        // "The number of material farmer loops is set by \"Number of sandwich rounds to run\".<br>"
        // "The number of Item Printer to Material Farmer loops is set by \"Number of rounds of Item Printer → Material farm\".",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , NUM_ROUNDS_OF_ITEM_PRINTER_TO_MATERIAL_FARM(
        "<b>Number of rounds of Item Printer → Material farm:</b><br>"
        "One round is: Using the item printer (looping through the table for as many rounds as you specify below), "
        "then farming materials at North Provice area 3, "
        "then flying back to the item printer.<br>"
        "Automatic Material Farming (see above) must be enabled",
        // (as per \"Number of rounds per Item Printer session\" below)
        //(as per \"Number of sandwich rounds\" below, under \"Material Farmer\")
        LockMode::UNLOCK_WHILE_RUNNING,
        3
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
    , MATERIAL_FARMER_DISABLED_EXPLANATION("")
    , MATERIAL_FARMER_OPTIONS(
        &LANGUAGE,
        NOTIFICATION_STATUS_UPDATE,
        NOTIFICATION_PROGRAM_FINISH,
        NOTIFICATION_ERROR_RECOVERABLE,
        NOTIFICATION_ERROR_FATAL
    )
{
    PA_ADD_OPTION(LANGUAGE);

    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(AUTO_MATERIAL_FARMING);
        PA_ADD_OPTION(NUM_ROUNDS_OF_ITEM_PRINTER_TO_MATERIAL_FARM);
    }
    PA_ADD_OPTION(NUM_ITEM_PRINTER_ROUNDS);
    PA_ADD_OPTION(AFTER_ITEM_PRINTER_DONE_EXPLANATION);
//    PA_ADD_OPTION(DATE0);
//    PA_ADD_OPTION(DATE1);
    PA_ADD_OPTION(TABLE);
    PA_ADD_OPTION(DELAY_MILLIS);
    PA_ADD_OPTION(ADJUST_DELAY);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(FIX_TIME_WHEN_DONE);

    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(MATERIAL_FARMER_DISABLED_EXPLANATION);
        PA_ADD_OPTION(MATERIAL_FARMER_OPTIONS);
    }

    PA_ADD_OPTION(NOTIFICATIONS);

    ItemPrinterRNG::value_changed(this);
    AUTO_MATERIAL_FARMING.add_listener(*this);
}

void ItemPrinterRNG::value_changed(void* object){
    ConfigOptionState state = AUTO_MATERIAL_FARMING ? ConfigOptionState::ENABLED : ConfigOptionState::DISABLED;
    MATERIAL_FARMER_OPTIONS.set_visibility(state);
    NUM_ROUNDS_OF_ITEM_PRINTER_TO_MATERIAL_FARM.set_visibility(state);
    if (AUTO_MATERIAL_FARMING){
        AFTER_ITEM_PRINTER_DONE_EXPLANATION.set_text("Then proceed to material farming.");
        MATERIAL_FARMER_DISABLED_EXPLANATION.set_text("<br>");
    }else{
        AFTER_ITEM_PRINTER_DONE_EXPLANATION.set_text("Then stop the program.");
        MATERIAL_FARMER_DISABLED_EXPLANATION.set_text("<br>To enable the Material Farmer, enable \"Automatic Material Farming\" above");
    }
}


void ItemPrinterRNG::run_print_at_date(
    SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    const DateTime& date, ItemPrinterJobs jobs
){
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
            std::array<std::string, 10> print_results = item_printer_finish_print(env.inference_dispatcher(), env.console, context, LANGUAGE);
            uint64_t seed = to_seconds_since_epoch(date);
            int distance_from_target = get_distance_from_target(env.console, print_results, seed);
            if (distance_from_target == 0){
                stats.frame_hits++;
            }else{
                stats.frame_misses++;
            }
            env.update_stats();
            if (ADJUST_DELAY){
                adjust_delay(env.logger(), env, print_results, distance_from_target);
            }

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

void ItemPrinterRNG::adjust_delay(
    Logger& logger,
    SingleSwitchProgramEnvironment& env,
    const std::array<std::string, 10>& print_results,
    int distance_from_target
){
    const int16_t ADJUSTMENT_TABLE[] = {0, 50, 1000};

    int16_t delay_adjustment;
    if (distance_from_target < 0){
        delay_adjustment = ADJUSTMENT_TABLE[-distance_from_target];
    }else if (distance_from_target > 0){
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
    const std::array<std::string, 10>& print_results, 
    uint64_t seed
){
    int distance_from_target = std::numeric_limits<int>::min();
    const int MAX_DEVIATION = 2;
    for (int current_deviation = 0; current_deviation < MAX_DEVIATION; current_deviation++){
        DateSeed seed_data = get_date_seed(seed - current_deviation);
        if (results_approximately_match(print_results, seed_data.items)){
            distance_from_target = -current_deviation;
            break;
        }
        if (results_approximately_match(print_results, seed_data.balls)){
            distance_from_target = -current_deviation;
            break;
        }

        if (current_deviation == 0){
            continue;
        }

        seed_data = get_date_seed(seed + current_deviation);
        if (results_approximately_match(print_results, seed_data.items)){
            distance_from_target = current_deviation;
            break;
        }
        if (results_approximately_match(print_results, seed_data.balls)){
            distance_from_target = current_deviation;
            break;
        }
    }

    if (distance_from_target < 0){
        logger.log("Frame Result: Missed. (Target - " + std::to_string(-distance_from_target) + ")", COLOR_ORANGE);
    }else if (distance_from_target > 0){
        logger.log("Frame Result: Missed. (Target + " + std::to_string(distance_from_target) + ")", COLOR_ORANGE);
    }else{
        logger.log("Frame Result: Target hit", COLOR_BLUE);
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

void ItemPrinterRNG::run_item_printer_rng(
    SingleSwitchProgramEnvironment& env, BotBaseContext& context, 
    ItemPrinterRNG_Descriptor::Stats& stats
){
    for (uint32_t c = 0; c < NUM_ITEM_PRINTER_ROUNDS; c++){
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

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
}

void ItemPrinterRNG::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);
    ItemPrinterRNG_Descriptor::Stats& stats = env.current_stats<ItemPrinterRNG_Descriptor::Stats>();
    env.update_stats();

    try{
        if (!AUTO_MATERIAL_FARMING){
            run_item_printer_rng(env, context, stats);
        }else{
            // Throw user setup errors early in program
            // - Ensure language is set
            const Language language = LANGUAGE;
            if (language == Language::None) {
                throw UserSetupError(env.console.logger(), "Must set game language option to read item printer rewards.");
            }

            // - Ensure audio input is enabled
            LetsGoKillSoundDetector audio_detector(env.console, [](float){ return true; });
            wait_until(
                env.console, context,
                std::chrono::milliseconds(1100),
                {audio_detector}
            );
            audio_detector.throw_if_no_sound(std::chrono::milliseconds(1000));

            // Don't allow the material farmer stats to affect the Item Printer's stats.
            MaterialFarmerStats mat_farm_stats;// = env.current_stats<MaterialFarmer_Descriptor::Stats>();
            for (int i = 0; i < NUM_ROUNDS_OF_ITEM_PRINTER_TO_MATERIAL_FARM; i++){
                run_item_printer_rng(env, context, stats);
                press_Bs_to_back_to_overworld(env.program_info(), env.console, context);
                move_from_item_printer_to_material_farming(env, context);
                run_material_farmer(env, context, MATERIAL_FARMER_OPTIONS, mat_farm_stats);
                move_from_material_farming_to_item_printer(env, context);
            }
        }
    }catch (ProgramFinishedException&){}

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
