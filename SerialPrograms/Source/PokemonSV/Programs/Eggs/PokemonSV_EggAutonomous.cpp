/*  Egg Autonomous
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
//#include "CommonFramework/Exceptions/UnexpectedBattleException.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxEggDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_IvJudgeReader.h"
//#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Programs/Boxes/PokemonSV_BoxRoutines.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV/Programs/Eggs/PokemonSV_EggRoutines.h"
#include "PokemonSV/Programs/Boxes/PokemonSV_BoxRelease.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_SandwichRoutines.h"
#include "PokemonSV/Programs/AutoStory/PokemonSV_MenuOption.h"
#include "PokemonSV_EggAutonomous.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


EggAutonomous_Descriptor::EggAutonomous_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:EggAutonomous",
        STRING_POKEMON + " SV", "Egg Autonomous",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/EggAutonomous.md",
        "Automatically get meal power, fetch eggs from a picnic and hatch them.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct EggAutonomous_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_sandwiches(m_stats["Sandwiches"])
        , m_fetch_attempts(m_stats["Fetch Attempts"])
        , m_eggs(m_stats["Eggs"])
        , m_hatched(m_stats["Hatched"])
        , m_shinies(m_stats["Shinies"])
        , m_kept(m_stats["Kept"])
        , m_errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Sandwiches");
        m_display_order.emplace_back("Fetch Attempts");
        m_display_order.emplace_back("Eggs");
        m_display_order.emplace_back("Hatched");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Kept");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);

        m_aliases.emplace(STRING_POKEMON + " Kept", "Kept");
    }

    std::atomic<uint64_t>& m_sandwiches;
    std::atomic<uint64_t>& m_fetch_attempts;
    std::atomic<uint64_t>& m_eggs;
    std::atomic<uint64_t>& m_hatched;
    std::atomic<uint64_t>& m_shinies;
    std::atomic<uint64_t>& m_kept;
    std::atomic<uint64_t>& m_errors;
};
std::unique_ptr<StatsTracker> EggAutonomous_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



EggAutonomous::EggAutonomous()
    : GO_HOME_WHEN_DONE(false)
    , LANGUAGE(
        "<b>Game Language:</b>",
        IV_READER().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , LOCATION(
        "<b>Location:</b><br>The location to hatch eggs.",
        {
            {EggAutoLocation::ZeroGate,         "zero-gate",        "Zero Gate"},
            {EggAutoLocation::NorthLighthouse,  "north-lighthouse", "North Province (Area Three) Lighthouse"}
        },
        LockMode::LOCK_WHILE_RUNNING,
        EggAutoLocation::ZeroGate
    )
    , MAX_KEEPERS(
        "<b>Max Keepers:</b><br>Stop the program after keeping this many " + STRING_POKEMON + ". "
        "The program will put them into a box neighboring the current box.",
        LockMode::UNLOCK_WHILE_RUNNING,
        10, 1, 30
    )
    , AUTO_SAVING(
        "<b>Auto-Saving:</b><br>Automatically save the game to recover from crashes and allow eggs to be unhatched.<br>" +
        make_text_url(
            ONLINE_DOC_URL_BASE + "ComputerControl/blob/master/Wiki/Programs/PokemonSV/EggAutonomous.md#auto-saving-mode",
            "See the wiki for the full explanations of each mode."
        ),
        {
            {AutoSave::NoAutoSave, "none", "No auto-saving."},
            {AutoSave::AfterStartAndKeep, "start-and-keep", "Save at beginning and after keeping a baby."},
            {AutoSave::EveryBatch, "every-batch", "Save before every batch of 4 or 5 eggs."},
            {AutoSave::AfterFetchComplete, "after-fetch", "Save after all eggs have been fetched from picnic."}
        },
        LockMode::LOCK_WHILE_RUNNING,
        AutoSave::AfterStartAndKeep
    )
    , HAS_CLONE_RIDE_POKEMON(
        "<b>Cloned Ride Legendary 2nd in Party:</b><br>"
        "Ride legendary cannot be cloned after patch 1.0.1. To preserve the existing clone while hatching eggs, "
        "place it as second in party before starting the program.</b>"
        "The program will skip the first row of the current box when storing and hatching eggs, so you will need "
        "to fill the first row with " + STRING_POKEMON + " before running this program.",
        LockMode::LOCK_WHILE_RUNNING,
        false)
    , KEEP_BOX_LOCATION(
        "<b>Location of the Keep Box:</b><br>Which box to keep the shiny " + STRING_POKEMON + " and others that match the filters.",
        {
            {0, "left", "Left Box"},
            {1, "right", "Right Box"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        1
    )
    , FILTERS0(
        StatsHuntIvJudgeFilterTable_Label_Eggs,
        {
            .action = true,
            .shiny = true,
            .gender = true,
            .nature = true,
        }
    )
    , SAVE_DEBUG_VIDEO(
        "<b>Save debug videos to Switch:</b><br>"
        "Set this on to save a Switch video everytime an error occurs. You can send the video to developers to help them debug later.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_NONSHINY_KEEP(
        "Non-Shiny Keep",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs"}
    )
    , NOTIFICATION_SHINY(
        "Shiny Hatch",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , m_notification_noop("", false, false)
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_NONSHINY_KEEP,
        &NOTIFICATION_SHINY,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(SAVE_DEBUG_VIDEO);
    }
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(EGG_SANDWICH);
    PA_ADD_OPTION(LOCATION);
    PA_ADD_OPTION(MAX_KEEPERS);
    PA_ADD_OPTION(AUTO_SAVING);
    PA_ADD_OPTION(KEEP_BOX_LOCATION);
    PA_ADD_OPTION(FILTERS0);
    PA_ADD_OPTION(HAS_CLONE_RIDE_POKEMON);

    PA_ADD_OPTION(NOTIFICATIONS);
}


void EggAutonomous::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    //  Connect the controller.
    pbf_press_button(context, BUTTON_L, 10, 100);

    {
        // reset_position_to_flying_spot(env, context);
        // picnic_party_to_hatch_party(env, context);
        // hatch_eggs_full_routine(env, context, -1);

        // enter_box_system_from_overworld(env.program_info(), env.console, context);
        // for(int i = 0; i < 5; i++){
        //     process_one_baby(env, context, i, 5);
        // }

        // eat_egg_sandwich_at_picnic(env.program_info(), env.normal_inference_dispatcher(), env.console, context,
        //     EGG_SANDWICH_TYPE, SWEET_HERB_INDEX_BACKWARDS.current_value());
        // return;
    }

    if (AUTO_SAVING != AutoSave::NoAutoSave){
        save_game(env, context, true);
    }

    const size_t max_num_sandwiches = EGG_SANDWICH.MAX_NUM_SANDWICHES;
    m_num_sandwich_spent = 0;
    m_num_kept = 0;
    size_t consecutive_failures = 0;
    while (true){
        m_saved_after_fetched_eggs = false;
        m_in_critical_to_save_stage = false;

        // Do one iteration of the outmost loop of egg auto:
        // - Start at Aera Zero flying spot
        // - Go to front area of the observation station to start picnic
        // - Make sandwich to gain egg power
        // - Go to picnic basket to fetch eggs
        // - Go on ride to hatch eggs

        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        // Recoverable loop to fetch eggs:
        int num_party_eggs = -1;
        while (true){
            try{
                num_party_eggs = fetch_eggs_full_routine(env, context);
                break;
            }catch (ScreenshotException& e){
                if (handle_recoverable_error(
                    env, context,
                    NOTIFICATION_ERROR_RECOVERABLE,
                    e,
                    consecutive_failures
                )){
                    throw;
                }
            } // end try catch
        } // end recoverable loop to fetch eggs:

        // Recoverable loop to hatch eggs
        bool game_already_resetted = false;
        while (true){
            try{
                hatch_eggs_full_routine(env, context, num_party_eggs);
                consecutive_failures = 0;
                break;
            }catch (ProgramFinishedException&){
                env.update_stats();
                GO_HOME_WHEN_DONE.run_end_of_program(context);
                send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
                return;
            }catch (ScreenshotException& e){
                if (handle_recoverable_error(
                    env, context,
                    NOTIFICATION_ERROR_RECOVERABLE,
                    e, consecutive_failures
                )){
                    throw;
                }
                // After resetting the game, we don't know how many eggs in party
                num_party_eggs = -1;

                // If there is no save during egg hatching, then the game is reset to before fetching eggs
                // So we need to break out of the recoverable hatch egg routine loop
                if (m_saved_after_fetched_eggs == false){
                    env.log("No save during egg hatching routine. After this reset, we should start the egg fetching routine now.");
                    game_already_resetted = true;
                    break;
                }
            } // end try catch
        } // end recoverable loop to hatch eggs

        // If the program can't save-reload, then we cannot reload spent sandwich ingredients
        if (AUTO_SAVING == AutoSave::NoAutoSave){
            m_num_sandwich_spent++;
        }else if (m_saved_after_fetched_eggs){
            env.log("Game already saved during egg hatching routine, so we cannot reset game to reset sandwich.");
            // If we save after fetching eggs, then the save solidifies spent sandwich ingredients.
            m_num_sandwich_spent++;

            env.log("Saving game here so that we can reset sandwich later");
            save_game(env, context, true);
        }else if (game_already_resetted == false){
            // Nothing found in this iteration
            env.log("Resetting game since nothing found, saving sandwich ingredients.");
            reset_game(env.program_info(), env.console, context);
        }else{ // game_already_resetted == true
            env.log("Game reset back to egg fetching routine.");
        }

        if (m_num_sandwich_spent >= max_num_sandwiches){
            env.console.overlay().add_log("Max sandwich count: " + std::to_string(max_num_sandwiches), COLOR_PURPLE);
            env.log("Max num sandwiches reached: " + std::to_string(max_num_sandwiches), COLOR_PURPLE);
            break;
        }
        // end of one full picnic->hatch iteration
    } // end the full egg autonomous loop

    env.update_stats();
    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

// start at Area Zero flying spot, start picnic, make sandwich, then fetch eggs at basket.
int EggAutonomous::fetch_eggs_full_routine(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    auto& stats = env.current_stats<EggAutonomous_Descriptor::Stats>();

    if (LOCATION == EggAutoLocation::ZeroGate) {
        picnic_at_zero_gate(env.program_info(), env.console, context);
    } else {
        pbf_press_button(context, BUTTON_L, 50, 40);
        picnic_from_overworld(env.program_info(), env.console, context);
    }
    // Now we are at picnic. We are at one end of picnic table while the egg basket is at the other end
    bool can_make_sandwich = eat_egg_sandwich_at_picnic(env, env.console, context,
        EGG_SANDWICH.EGG_SANDWICH_TYPE, LANGUAGE);
    if (can_make_sandwich == false){
        throw UserSetupError(env.console, "No sandwich recipe or ingredients. Cannot open and select the sandwich recipe.");
    }
    env.current_stats<EggAutonomous_Descriptor::Stats>().m_sandwiches++;
    env.update_stats();

    // move past the table and collect eggs
    auto basket_check_callback = [&](size_t new_eggs){
        stats.m_fetch_attempts++;
        stats.m_eggs += new_eggs;
        env.update_stats();
    };

    const size_t max_eggs = HAS_CLONE_RIDE_POKEMON ? 24 : 30;
    size_t num_eggs_collected = 0;
    const size_t basket_wait_seconds = (EGG_SANDWICH.EGG_SANDWICH_TYPE == EggSandwichType::GREAT_PEANUT_BUTTER ? 180 : 120);
    collect_eggs_after_sandwich(env.program_info(), env.console, context, basket_wait_seconds, max_eggs,
        num_eggs_collected, basket_check_callback);

    leave_picnic(env.program_info(), env.console, context);

    // Reset position to flying spot:
    if (LOCATION == EggAutoLocation::ZeroGate) {
        reset_position_to_flying_spot(env, context);
    } else {
        //Lighthouse: We haven't moved much so just fly.
        open_map_from_overworld(env.program_info(), env.console, context);
        fly_to_overworld_from_map(env.program_info(), env.console, context);
    }

    return picnic_party_to_hatch_party(env, context);
}

// The routine to hatch a box of eggs after fetching.
// When there is an error during hatching, it will throw an OperationFailedException.
// The outmost program loop should catch this exception and recall this function to achieve error recovery, if needed.
// When immediately called after fetching routine finishes, assumes the party is already loaded with the first column of
// eggs and the number of eggs is provided by `num_eggs_in_party`.
// When called after an error recovery, assumes the party is loaded with some unknown number of eggs, and `num_eggs_in_party` is -1.
void EggAutonomous::hatch_eggs_full_routine(SingleSwitchProgramEnvironment& env, ProControllerContext& context, int num_eggs_in_party){
    auto& stats = env.current_stats<EggAutonomous_Descriptor::Stats>();

    // index of the egg column in box to be loaded next, 0-indexed
    // starting at 1 because when hatch_eggs_full_routine() is called, the first column is already loaded to the party
    uint8_t next_egg_column = 1;

    // Find the next egg column in the current box.
    // Note: the box columns can be:
    // - empty. This is due to the column loaded to party, or released in the case of AutoSaving != AfterStartAndKeep.
    // - egg column.
    // - hatched pokemon. This only happens in the case of AutoSaving == AfterStartAndKeep.
    // To handle all AutoSaving cases, we cannot only use `SomethingInBoxSlotDetector` to find the column, but also need
    // `BoxCurrentEggDetector`. 
    // This function is called in two cases:
    // 1. When we recover from error and don't know which column is the next egg column.
    //    In this case, `change_stats_view_to_judge()` is called as part of the error recovery code before this function,
    //    so that `BoxCurrentEggDetector` in this function can function correctly to find that the egg column.
    // 2. When we finish processing one hatched party and want to load the next egg column.
    //    In this case, `next_egg_column` already point to the next possible column, that cannot be hatched pokemon.
    //    So we don't need to call egg detector and don't need to worry about setting box views.
    auto go_to_next_egg_column = [&](bool check_eggs = false){
        SomethingInBoxSlotDetector sth_in_box_detector(COLOR_RED);
        BoxCurrentEggDetector egg_detector;
        for (; next_egg_column < 6; next_egg_column++){
            move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::SLOTS, HAS_CLONE_RIDE_POKEMON ? 1 : 0, next_egg_column);
            context.wait_for_all_requests();
            auto snapshot = env.console.video().snapshot();
            // If there is an egg in slot row 0 (or 1 if using clone ride pokemon), col `col`,
            if (sth_in_box_detector.detect(snapshot) && (!check_eggs || egg_detector.detect(snapshot))){
                env.log("Found next column of eggs at col " + std::to_string(next_egg_column) + ".");
                break;
            }
        }
    };

    if (num_eggs_in_party < 0){
        // detect how many eggs in party
        enter_box_system_from_overworld(env.program_info(), env.console, context);

        context.wait_for(std::chrono::milliseconds(400)); // wait until box UI is loaded

        // Move box cursor to party lead
        move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::PARTY, 0, 0);

        // Change box view to judge or stats
        Language language = LANGUAGE;
        if (language == Language::None){
            change_view_to_stats_or_judge(env.console, context);
        }else{
            change_view_to_judge(env.console, context, language);
        }

        const uint8_t expected_non_eggs_count_in_party = HAS_CLONE_RIDE_POKEMON ? 1 : 0;
        num_eggs_in_party = check_non_eggs_count_in_party(env.program_info(), env.console, context, expected_non_eggs_count_in_party);
        env.log("Read " + std::to_string(num_eggs_in_party) + " eggs.");
        env.console.overlay().add_log("Party eggs: " + std::to_string(num_eggs_in_party), COLOR_WHITE);

        // Also detect what's the next egg column
        context.wait_for(std::chrono::seconds(2)); // wait until box UI is definitely loaded
        env.log("Checking next egg column.");
        go_to_next_egg_column(true);

        leave_box_system_to_overworld(env.program_info(), env.console, context);
    }

    auto save_game_if_needed = [&](){
        if (AUTO_SAVING == AutoSave::EveryBatch ||
            (AUTO_SAVING == AutoSave::AfterFetchComplete && m_saved_after_fetched_eggs == false) ||
            (AUTO_SAVING == AutoSave::AfterStartAndKeep && m_in_critical_to_save_stage)){
            env.log("Saving game during egg hatching routine.");
            save_game(env, context, true);
            m_saved_after_fetched_eggs = true;
            m_in_critical_to_save_stage = false;
        }
    };

    // The loop to hatch batches of eggs.
    // Each batch consists of at most five eggs.
    // There are at most six batches of eggs in a box.
    while(true){
        save_game_if_needed();

        auto hatched_callback = [&](uint8_t){
            stats.m_hatched++;
            env.update_stats();
        };
        if (LOCATION == EggAutoLocation::ZeroGate) {
            hatch_eggs_at_zero_gate(env.program_info(), env.console, context, (uint8_t)num_eggs_in_party, hatched_callback);
            reset_position_to_flying_spot(env, context);
        } else {
            hatch_eggs_at_area_three_lighthouse(env.program_info(), env.console, context, (uint8_t)num_eggs_in_party, hatched_callback);
            reset_position_to_flying_spot(env, context);
            //Clear spawns - over time floette/vivillon drift over past the fence (usually aroudn the 3rd batch)
            picnic_from_overworld(env.program_info(), env.console, context);
            leave_picnic(env.program_info(), env.console, context);
        }

        enter_box_system_from_overworld(env.program_info(), env.console, context);
        
        // Check each hatched baby whether they will be kept.
        // If yes, move them to the keep box.
        // Otherwise, release them or move them into box in case we will reset game later.
        for(uint8_t i = 0; i < num_eggs_in_party; i++){
            process_one_baby(env, context, i, (uint8_t)num_eggs_in_party);
        }

        // If the auto save mode is AfterStartAndKeep, which allows resetting the game in case no eggs in the box are kept,
        // then we can save the time of releasing hatched pokemon in case we will reset the game later.
        // So here we place the hatched pokemon back to the box
        if (AUTO_SAVING == AutoSave::AfterStartAndKeep){
            // move party back into the box
            env.log("Unload party in case we will reset game later to save releasing time.");
            unload_one_column_from_party(env, env.console, context, NOTIFICATION_ERROR_RECOVERABLE, next_egg_column-1, HAS_CLONE_RIDE_POKEMON);
        }

        // Get the next egg column
        go_to_next_egg_column();

        if (next_egg_column < 6){
            load_one_column_to_party(env, env.console, context, NOTIFICATION_ERROR_RECOVERABLE, next_egg_column, HAS_CLONE_RIDE_POKEMON);
            // Move cursor to party lead so that we can examine rest of party to detect eggs.
            move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::PARTY, 0, 0);

            uint8_t expected_non_eggs_count_in_party = HAS_CLONE_RIDE_POKEMON ? 1 : 0;
            num_eggs_in_party = check_non_eggs_count_in_party(env.program_info(), env.console, context, expected_non_eggs_count_in_party);
        }else{
            // no more eggs to hatch in box

            if (AUTO_SAVING == AutoSave::AfterStartAndKeep){
                // Check if we will reset game:
                // We reset game if the auto save mode is AfterStartAndKeep and we have no pokemon kept during hatching this box
                // m_in_critical_to_save_stage: whether we need to save the game for the current batch of eggs
                // m_saved_after_fetched_eggs: whether we have saved the game already for a past batch of eggs in this box
                if (m_in_critical_to_save_stage == false && m_saved_after_fetched_eggs == false){
                    // Yes, we will reset game
                    // So exit this loop early here
                    env.log("Early exit of egg hatching routine to reset game.");
                    return;
                }

                // release the hatched pokemon in box
                size_t local_errors = 0;
                release_box(
                    env.program_info(), env.console, context, local_errors,
                    HAS_CLONE_RIDE_POKEMON ? 1 : 0
                );
            }

            // change to fetching mode:
            env.log("Replace party with picnic team");
            env.console.overlay().add_log("Change to picnic pokemon", COLOR_WHITE);

            // Move to left box
            move_to_left_box(context);

            // Swap the party lead, the flame body pokemon with the stored first fetching pokemon
            swap_two_box_slots(env.program_info(), env.console, context,
                BoxCursorLocation::PARTY, 0, 0,
                BoxCursorLocation::SLOTS, 0, 0);

            // Load rest of the fetching pokemon to party
            load_one_column_to_party(env, env.console, context, NOTIFICATION_ERROR_RECOVERABLE, 1, HAS_CLONE_RIDE_POKEMON);

            // Move back to middle box
            move_to_right_box(context);
        }

        leave_box_system_to_overworld(env.program_info(), env.console, context);

        if (next_egg_column == 6){ // no more eggs to hatch
            break; // break egg batch loop. This is the only place to break out of the loop
        }
        next_egg_column++;
    } // end egg batch loop

    save_game_if_needed();
}

// While in box system and the current box is egg box, process one baby pokemon in party
// Return true if the program finds a pokemon to keep
void EggAutonomous::process_one_baby(SingleSwitchProgramEnvironment& env, ProControllerContext& context, uint8_t egg_index, uint8_t num_eggs_in_party){
    auto& stats = env.current_stats<EggAutonomous_Descriptor::Stats>();

    // Check each pokemon from bottom to top. In this way we can reliably detect end of releasing the pokemon.
    uint8_t party_row = num_eggs_in_party - egg_index + (HAS_CLONE_RIDE_POKEMON ? 1 : 0);
    context.wait_for_all_requests();
    move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::PARTY, party_row, 0);

    env.log("Check hatched pokemon at party slot " + std::to_string(party_row));

    bool found_shiny = false;
    StatsHuntAction action = StatsHuntAction::Discard;
    if (check_baby_info(env.program_info(), env.console, context, LANGUAGE, FILTERS0, action)){
        found_shiny = true;
        env.console.log("Shiny found!");
        env.console.overlay().add_log("Shiny " + std::to_string(egg_index+1) + "/" + std::to_string(num_eggs_in_party), COLOR_GREEN);
        stats.m_shinies++;
        env.update_stats();
        send_encounter_notification(
            env,
            m_notification_noop,
            NOTIFICATION_SHINY,
            false, true, {{{}, ShinyType::UNKNOWN_SHINY}}, std::nan(""),
            env.console.video().snapshot()
        );
    }else{
        env.console.overlay().add_log("Not shiny " + std::to_string(egg_index+1) + "/" + std::to_string(num_eggs_in_party), COLOR_WHITE);
    }

    auto send_keep_notification = [&](){
        if (!found_shiny){
            send_encounter_notification(
                env,
                NOTIFICATION_NONSHINY_KEEP,
                NOTIFICATION_SHINY,
                false, false, {}, std::nan(""),
                env.console.video().snapshot()
            );
        }
    };

    switch (action){
    case StatsHuntAction::StopProgram:
            env.log("Program stop requested...");
            env.console.overlay().add_log("Request program stop", COLOR_WHITE);
            send_keep_notification();
            throw ProgramFinishedException();
    case StatsHuntAction::Keep:
            m_in_critical_to_save_stage = true;
            env.log("Moving Pokemon to keep box...", COLOR_BLUE);
            stats.m_kept++;
            env.update_stats();
            m_num_kept++;
            env.console.overlay().add_log("Keep pokemon " + std::to_string(m_num_kept) + "/" + std::to_string(MAX_KEEPERS), COLOR_YELLOW);
            send_keep_notification();

            if (move_pokemon_to_keep(env, context, party_row) == false){
                env.log("No empty slot available to place new pokemon.");
                env.console.overlay().add_log("No box space", COLOR_RED);
                throw ProgramFinishedException();
            }

            if (m_num_kept >= MAX_KEEPERS){
                env.log("Max keepers reached. Stopping program...");
                env.console.overlay().add_log("Max Keepers reached", COLOR_WHITE);
                throw ProgramFinishedException();
            }
            break;

    case StatsHuntAction::Discard:
        default:
            // If the auto save mode is AfterStartAndKeep, which allows resetting the game in case no eggs in the box are kept,
            // then we can save the time of releasing hatched pokemon in case we will reset the game later.
            // Otherwise, release the pokemon now.
            if (AUTO_SAVING != AutoSave::AfterStartAndKeep){
                size_t local_errors = 0;
                release_one_pokemon(env.program_info(), env.console, context, local_errors);
            }
            break;
    } // end switch EggHatchAction
}

// From the egg box, move to the kept box, drop the pokemon to an empty spot in the box, move back to the egg box.
// Return false if it does not find an empty spot.
bool EggAutonomous::move_pokemon_to_keep(SingleSwitchProgramEnvironment& env, ProControllerContext& context, uint8_t pokemon_row_in_party){
    SomethingInBoxSlotDetector sth_in_box_detector(COLOR_RED);
    const size_t keep_box_location = KEEP_BOX_LOCATION.current_value();
    if (keep_box_location == 0){
        move_to_left_box(context);
    }else{
        move_to_right_box(context);
    }
    
    context.wait_for_all_requests();

    const uint8_t col_start = (keep_box_location == 0 ? 2 : 0);
    for (uint8_t row = 0; row < 5; row++){
        for (uint8_t col = col_start; col < 6; col++){
            move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::SLOTS, row, col);
            context.wait_for_all_requests();
            // If no pokemon in the slot:
            if (!sth_in_box_detector.detect(env.console.video().snapshot())){

                // Move the to-keep pokemon in party to the empty slot.
                swap_two_box_slots(env.program_info(), env.console, context,
                    BoxCursorLocation::PARTY, pokemon_row_in_party, 0,
                    BoxCursorLocation::SLOTS, row, col);

                // Move back to middle box
                if (keep_box_location == 0){
                    move_to_right_box(context);
                }else{
                    move_to_left_box(context);
                }
                context.wait_for_all_requests();
                return true;
            }
        }
    }
    return false;
}

void EggAutonomous::reset_position_to_flying_spot(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // Use map to fly back to the flying spot
    open_map_from_overworld(env.program_info(), env.console, context);
    if (LOCATION == EggAutoLocation::ZeroGate) {
        pbf_move_left_joystick(context, 128, 160, 20, 50);
    } else { //lighthouse
        pbf_move_left_joystick(context, 130, 0, 150ms, 50ms);
        pbf_press_button(context, BUTTON_ZL, 40, 100);
    }
    fly_to_overworld_from_map(env.program_info(), env.console, context);
}

// From overworld, change pokemon party from the one used for getting eggs, to the one used for hatching
// The new party will be one flame body pokemon as lead, and some eggs.
// Function returns how many eggs are in the party
int EggAutonomous::picnic_party_to_hatch_party(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    env.console.overlay().add_log("Change to hatching", COLOR_WHITE);
    // change pokemon party from used for fetching eggs in picnic, to hatching on your ride.

    enter_box_system_from_overworld(env.program_info(), env.console, context);

    // Move to left box
    move_to_left_box(context);

    // Swap the stored the flame body pokemon with the stored first fetching pokemon
    swap_two_box_slots(env.program_info(), env.console, context,
        BoxCursorLocation::SLOTS, 0, 0,
        BoxCursorLocation::PARTY, 0, 0);

    // Unload rest of party to the 2nd column (col 1) in box
    unload_one_column_from_party(env, env.console, context, NOTIFICATION_ERROR_RECOVERABLE, 1, HAS_CLONE_RIDE_POKEMON);

    // Move to middle box
    move_to_right_box(context);

    // Load first egg column to party
    load_one_column_to_party(env, env.console, context, NOTIFICATION_ERROR_RECOVERABLE, 0, HAS_CLONE_RIDE_POKEMON);
    // Move cursor to party lead so that we can examine rest of party to detect eggs.
    move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::PARTY, 0, 0);

    uint8_t expected_non_eggs_count_in_party = HAS_CLONE_RIDE_POKEMON ? 1 : 0;
    const uint8_t num_eggs_in_party = check_non_eggs_count_in_party(env.program_info(), env.console, context, expected_non_eggs_count_in_party);

    leave_box_system_to_overworld(env.program_info(), env.console, context);

    return num_eggs_in_party;
}


void EggAutonomous::save_game(SingleSwitchProgramEnvironment& env, ProControllerContext& context, bool from_overworld){
    try{
        if (from_overworld){
            save_game_from_overworld(env.program_info(), env.console, context);
        }else{
            save_game_from_menu(env.program_info(), env.console, context);
        }
    }catch (OperationFailedException& e){
        // To be safe: avoid interrupting or corrupting game saving,
        // make game saving non error recoverable
        throw FatalProgramException(std::move(e));
    }
}

void change_settings_egg_program(SingleSwitchProgramEnvironment& env, ProControllerContext& context,  Language language){
    int8_t options_index = 4;
    enter_menu_from_overworld(env.program_info(), env.console, context, options_index, MenuSide::RIGHT);
    MenuOption session(env.console, context, language);

    std::vector<std::pair<MenuOptionItemEnum, std::vector<MenuOptionToggleEnum>>> options = {
        {MenuOptionItemEnum::TEXT_SPEED, {MenuOptionToggleEnum::FAST}},
        {MenuOptionItemEnum::SKIP_MOVE_LEARNING, {MenuOptionToggleEnum::ON}},
        {MenuOptionItemEnum::GIVE_NICKNAMES, {MenuOptionToggleEnum::OFF}},
        {MenuOptionItemEnum::CAMERA_SUPPORT, {MenuOptionToggleEnum::ON}},
        {MenuOptionItemEnum::AUTOSAVE, {MenuOptionToggleEnum::OFF}},

    };
    session.set_options(options); 

    pbf_mash_button(context, BUTTON_A, 1 * TICKS_PER_SECOND);
    clear_dialog(env.console, context, ClearDialogMode::STOP_TIMEOUT, 5, {CallbackEnum::PROMPT_DIALOG});
    press_Bs_to_back_to_overworld(env.program_info(), env.console, context);    
}


bool EggAutonomous::handle_recoverable_error(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    EventNotificationOption& notification,
    const ScreenshotException& e,
    size_t& consecutive_failures
){
    auto& stats = env.current_stats<EggAutonomous_Descriptor::Stats>();
    stats.m_errors++;
    env.update_stats();

    if (SAVE_DEBUG_VIDEO){
        // Take a video to give more context for debugging
        pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
        context.wait_for_all_requests();
    }
    // if there is no auto save, then we shouldn't reset game to lose previous progress.
    if (AUTO_SAVING == AutoSave::NoAutoSave){
        return true;
    }

    if (AUTO_SAVING == AutoSave::AfterStartAndKeep && m_in_critical_to_save_stage){
        // We have found a pokemon to keep, but before we can save the game to protect the pokemon, an error occurred.
        // To not lose the pokemon, don't reset.
        // Note: if AUTO_SAVING == AutoSave::EveryBatch, then we don't need to care about this critical stage, because
        // in this auto saving mode, every batch of eggs have been saved beforehand.
        env.log("Found an error before we can save the game to protect the newly kept pokemon.", COLOR_RED);
        env.log("Don't reset game to protect it.", COLOR_RED);
        return true;
    }
    std::string fail_message = e.message();
    consecutive_failures++;
    if (consecutive_failures >= 3){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed 3 times in the row.\n" + fail_message,
            env.console
        );
    }
    e.send_recoverable_notification(env);

    env.log("Reset game to handle recoverable error");
    reset_game(env.program_info(), env.console, context);

    if (e.message().find("collect_eggs_from_basket") != std::string::npos){
        change_settings_egg_program(env, context, LANGUAGE);
    }

    return false;
}


}
}
}
