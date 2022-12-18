/*  Egg Autonomous
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSV/Inference/PokemonSV_EggDetector.h"
#include "PokemonSV/Inference/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/PokemonSV_MapDetector.h"
#include "PokemonSV/Inference/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/PokemonSV_IVCheckerReader.h"
#include "PokemonSV/Inference/PokemonSV_BoxGenderDetector.h"
#include "PokemonSV/Inference/PokemonSV_BoxShinyDetector.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Programs/Boxes/PokemonSV_BoxRoutines.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Programs/Eggs/PokemonSV_EggRoutines.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_SandwichRoutines.h"
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
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct EggAutonomous_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_sandwiches(m_stats["Sandwiches"])
        , m_fetch_attempts(m_stats["Fetch Attempts"])
        , m_eggs(m_stats["Eggs"])
        , m_hatched(m_stats["Hatched"])
        , m_shinies(m_stats["Shinies"])
        , m_kept(m_stats[STRING_POKEMON + " Kept"])
        , m_errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Sandwiches");
        m_display_order.emplace_back("Fetch Attempts");
        m_display_order.emplace_back("Eggs");
        m_display_order.emplace_back("Hatched");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back(STRING_POKEMON + " Kept");
        m_display_order.emplace_back("Errors", true);
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
    , MAX_NUM_SANDWICHES(
        "<b>Max num sandwiches:</b><br>How many " + STRING_POKEMON + " Great Peanut Butter Sandwich you can make before running out of ingredients.",
        LockWhileRunning::LOCKED,
        100, 0
    )
    , LANGUAGE(
        "<b>Game Language:</b><br>Required to read IVs.",
        IVCheckerReader::instance().languages(),
        LockWhileRunning::LOCKED,
        false
    )
    , MAX_KEEPERS(
        "<b>Max Keepers:</b><br>Stop the program after keeping this many " + STRING_POKEMON + ". "
        "This number plus the number of " + STRING_POKEMON + " in the box left to your current box must not exceed 30. "
        "Otherwise, the program will break when that box is full.",
        LockWhileRunning::LOCKED,
        10, 1, 30
    )
    , AUTO_SAVING(
        "<b>Auto-Saving:</b><br>Automatically save the game to recover from crashes and allow eggs to be unhatched.<br>"
        "No auto-saving: No error/crash recovery. No sandwich ingredients permanently spent.<br>"
        "Save before picnic and after keeping a baby: Allows for error/crash recovery. Ingredients permanently spent after baby kept.<br>"
        "Save before every batch: Allows you to unhatch eggs. Ingredients permanently spent after every picnic.<br><br>"
        "Unhatching eggs can be useful for obtaining breeding parents by rehatching a perfect egg in a game with a different language.<br>"
        "To collect (unhatched) eggs with the desired stats, set this option to \"Save before every batch\". "
        "Then set the Action Table below to \"Stop Program\" on the desired stats. "
        "Once the program stops on the baby with the desired stats, you can manually reset the game and it will revert to an egg in your party.",
        {
            {AutoSave::NoAutoSave, "none", "No auto-saving."},
            {AutoSave::AfterStartAndKeep, "start-and-keep", "Save at beginning and after keeping a baby."},
            {AutoSave::EveryBatch, "every-batch", "Save before every batch."},
        },
        LockWhileRunning::LOCKED,
        AutoSave::AfterStartAndKeep
    )
    , SAVE_DEBUG_VIDEO(
        "<b>Save debug videos to Switch:</b>",
        LockWhileRunning::LOCKED,
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
    // Program requirement:
    // - At Area Zero flying spot
    // - Text Fast
    // - unlock sandwich No. 17
    // - has enough ingredients to make sandwich No. 17
    // - Current box is empty, to hold eggs
    // - The box to the left of the current box has empty slots for found baby pokemon (shiny or matching user defined requirements)
    // - The box to the right of the current box has a flame body pokemon at row 0, col 0 (0-indexed).
    // - the second leftmost column of the box to the right of the current box is empty. It is used to place the hatching party column
    // - box in judge view
    // - What to do when party pokemon gain exp after sandwich?

    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(MAX_NUM_SANDWICHES);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(MAX_KEEPERS);
    PA_ADD_OPTION(AUTO_SAVING);
    PA_ADD_OPTION(FILTERS);

    PA_ADD_OPTION(NOTIFICATIONS);

    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(SAVE_DEBUG_VIDEO);
    }
}


void EggAutonomous::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 10, 0);

    {
        // make_great_peanut_butter_sandwich(env, env.console, context);
        // reset_game(env, context, "reset");
        // fetch_eggs_full_routine(env, context);
        // reset_position_to_flying_spot(env, context);
        // picnic_party_to_hatch_party(env, context);
        // hatch_eggs_full_routine(env, context, -1);
        // return;
    }
    
    if (AUTO_SAVING != AutoSave::NoAutoSave){
        save_game(env, context, true);
    }

    m_num_sandwich_spent = 0;
    m_num_kept = 0;
    m_saved_after_fetched_eggs = false;

    while(true){

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
        while(true){
            size_t consecutive_failures = 0;
            try {
                num_party_eggs = fetch_eggs_full_routine(env, context);
                break;
            } catch(OperationFailedException& e){
                handle_recoverable_error(env, context, e, consecutive_failures);
            } // end try catch
        } // end recoverable loop to fetch eggs:

        // Recoverable loop to hatch eggs
        bool game_already_resetted = false;
        while(true){
            size_t consecutive_failures = 0;
            try {
                hatch_eggs_full_routine(env, context, num_party_eggs);
                break;
            } catch(OperationFailedException& e){
                handle_recoverable_error(env, context, e, consecutive_failures);
                // After resetting the game, we don't know how many eggs in party
                num_party_eggs = -1;

                // If there is no save during egg hatching, then the game is reset to before fetching eggs
                // So we need to break out of the recoverable hatch egg routine loop
                if (m_saved_after_fetched_eggs == false){
                    game_already_resetted = true;
                    break;
                }
            } catch (ProgramFinishedException&){
                env.update_stats();
                send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
                return;
            } // end try catch
        } // end recoverable loop to hatch eggs

        // If the program can't save-reload, then we cannot reload spent sandwich ingredients
        if (AUTO_SAVING == AutoSave::NoAutoSave){
            m_num_sandwich_spent++;
        } else if (m_saved_after_fetched_eggs){
            // If we save after fetching eggs, then the save solidifies spent sandwich ingredients.
            m_num_sandwich_spent++;
            m_saved_after_fetched_eggs = false;
        } else if (game_already_resetted == false){
            // Nothing found in this iteration
            env.log("Resetting game since nothing found, saving sandwich ingredients.");
            reset_game(env, context, "reset to start new meal");
        }

        if (m_num_sandwich_spent >= MAX_NUM_SANDWICHES){
            env.console.overlay().add_log("Max sandwich count: " + std::to_string(MAX_NUM_SANDWICHES), COLOR_PURPLE);
            env.log("Max num sandwiches reached: " + std::to_string(MAX_NUM_SANDWICHES), COLOR_PURPLE);
            break;
        }
        // end of one full picnic->hatch iteration
    } // end the full egg autonomous loop

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

// start at Area Zero flyting spot, start picnic, make sandwich, then fetch eggs at basket.
int EggAutonomous::fetch_eggs_full_routine(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    auto& stats = env.current_stats<EggAutonomous_Descriptor::Stats>();

    picnic_at_zero_gate(env.program_info(), env.console, context);
    // Now we are at picnic. We are at one end of picnic table while the egg basket is at the other end
    
    bool can_make_sandwich = eat_egg_sandwich_at_picnic(env.program_info(), env.realtime_dispatcher(), env.console, context);
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

    const size_t max_eggs = 30;
    size_t num_eggs_collected = 0;
    collect_eggs_after_sandwich(env.program_info(), env.console, context, max_eggs, num_eggs_collected, basket_check_callback);

    leave_picnic(env.program_info(), env.console, context);

    // Reset position to flying spot:
    reset_position_to_flying_spot(env, context);

    return picnic_party_to_hatch_party(env, context);
}

void EggAutonomous::hatch_eggs_full_routine(SingleSwitchProgramEnvironment& env, BotBaseContext& context, int num_eggs_in_party){    
    auto& stats = env.current_stats<EggAutonomous_Descriptor::Stats>();
    if (num_eggs_in_party < 0){
        // detect how many eggs in party
        enter_box_system_from_overworld(env.program_info(), env.console, context);
    
        context.wait_for(std::chrono::milliseconds(400)); // wait until box UI fully loaded

        num_eggs_in_party = check_only_eggs_in_party(env.program_info(), env.console, context);
        env.log("Read " + std::to_string(num_eggs_in_party) + " eggs.");
        env.console.overlay().add_log("Party eggs: " + std::to_string(num_eggs_in_party), COLOR_WHITE);

        leave_box_system_to_overworld(env.program_info(), env.console, context);
    }

    bool need_to_save_after_kept = false;
    // The loop to hatch batches of eggs.
    // Each batch consists of at most five eggs.
    // There are at most six batches of eggs in a box.
    uint8_t next_egg_column = 0; // next egg column in box
    while(true){
        if (AUTO_SAVING == AutoSave::EveryBatch || (AUTO_SAVING == AutoSave::AfterStartAndKeep && need_to_save_after_kept)){
            save_game(env, context, true);
            m_saved_after_fetched_eggs = true;
            need_to_save_after_kept = false;
        }

        auto hatched_callback = [&](uint8_t){  
            stats.m_hatched++;
            env.update_stats();
        };
        hatch_eggs_at_zero_gate(env.program_info(), env.console, context, (uint8_t)num_eggs_in_party, hatched_callback);
        reset_position_to_flying_spot(env, context);

        enter_box_system_from_overworld(env.program_info(), env.console, context);
        
        for(int i = 0; i < num_eggs_in_party; i++){
            if (process_one_baby(env, context, i, num_eggs_in_party)){
                need_to_save_after_kept = true;
            }
        } // end for each hatched pokemon in party

        // Get the next egg column
        SomethingInBoxSlotDetector sth_in_box_detector(COLOR_RED);
        for (; next_egg_column < 6; next_egg_column++){
            move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::SLOTS, 0, next_egg_column);
            context.wait_for_all_requests();
            // If there is pokemon in slot row 0, col `col`,
            if (sth_in_box_detector.detect(env.console.video().snapshot())){
                env.log("Found next column of eggs at col " + std::to_string(next_egg_column));
                env.console.overlay().add_log("Add next column", COLOR_WHITE);
                break;
            }
        }
        context.wait_for_all_requests();
        if (next_egg_column < 6){
            load_one_column_to_party(env.program_info(), env.console, context, next_egg_column);
            // Move cursor to party lead so that we can examine rest of party to detect eggs.
            move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::PARTY, 0, 0);
            
            num_eggs_in_party = check_only_eggs_in_party(env.program_info(), env.console, context);
        } else {
            // no more eggs in box, change to fetching mode:
            env.log("Replace party with picnic team");
            env.console.overlay().add_log("Change to picnic pokemon", COLOR_WHITE);
            
            // Move to right box
            move_to_right_box(context);

            // Swap the party lead, the flame body pokemon with the stored first fetching pokemon
            swap_two_box_slots(env.program_info(), env.console, context,
                BoxCursorLocation::PARTY, 0, 0,
                BoxCursorLocation::SLOTS, 0, 0);
            
            // Load rest of the fetching pokemon to party
            load_one_column_to_party(env.program_info(), env.console, context, 1);
            
            // Move back to middle box
            move_to_left_box(context);
        }

        leave_box_system_to_overworld(env.program_info(), env.console, context);

        if (next_egg_column == 6){ // no more eggs to hatch
            if ((AUTO_SAVING == AutoSave::AfterStartAndKeep && need_to_save_after_kept) || AUTO_SAVING == AutoSave::EveryBatch){
                save_game(env, context, true);
                m_saved_after_fetched_eggs = true;
            }
            break; // break egg batch loop. This is the only place to break out of the loop
        }
        next_egg_column++;
    } // end egg batch loop
}

// While in box system and the current box is egg box, process one baby pokemon in party
// Return true if the program finds a pokemon to keep
bool EggAutonomous::process_one_baby(SingleSwitchProgramEnvironment& env, BotBaseContext& context, int egg_index, int num_eggs_in_party){
    auto& stats = env.current_stats<EggAutonomous_Descriptor::Stats>();

    // Check each pokemon from bottom to top. In this way we can reliably detect end of releasing the pokemon.
    const int party_row = num_eggs_in_party - egg_index;
    context.wait_for_all_requests();
    move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::PARTY, party_row, 0);

    env.log("Check hatched pokemon at party slot " + std::to_string(party_row+2));
    
    bool found_shiny = false;
    EggHatchAction action = EggHatchAction::Release;
    if (check_baby_info(env.console, context, LANGUAGE, FILTERS, action)){
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
        case EggHatchAction::StopProgram:
            env.log("Program stop requested...");
            env.console.overlay().add_log("Request program stop", COLOR_WHITE);
            send_keep_notification();
            throw ProgramFinishedException();
        case EggHatchAction::Keep:
            m_in_critical_to_save_stage = true;
            env.log("Moving Pokemon to keep box...", COLOR_BLUE);
            stats.m_kept++;
            env.update_stats();
            m_num_kept++;
            env.console.overlay().add_log("Keep pokemon " + std::to_string(m_num_kept) + "/" + std::to_string(MAX_KEEPERS), COLOR_YELLOW);
            send_keep_notification();

            if (move_pokemon_to_keep(env, context, 1) == false) {
                env.log("No empty slot availble to place new pokemon.");
                env.console.overlay().add_log("No box space", COLOR_RED);
                throw ProgramFinishedException();
            }

            if (m_num_kept >= MAX_KEEPERS){
                env.log("Max keepers reached. Stopping program...");
                env.console.overlay().add_log("Max Keepers reached", COLOR_WHITE);
                throw ProgramFinishedException();
            }
            break;
        
        case EggHatchAction::Release:
        default:
            release_one_pokemon(env.program_info(), env.console, context);
            break;
    } // end switch EggHatchAction
    return m_in_critical_to_save_stage;
}

// From the egg box, move left to the kept box, drop the pokemon to an empty spot in the box, move back to the egg box.
// Return false if it does not find an empty spot.
bool EggAutonomous::move_pokemon_to_keep(SingleSwitchProgramEnvironment& env, BotBaseContext& context, uint8_t pokemon_row_in_party){
    SomethingInBoxSlotDetector sth_in_box_detector(COLOR_RED);
    move_to_left_box(context);
    context.wait_for_all_requests();

    for (uint8_t row = 0; row < 5; row++){
        for (uint8_t col = 0; col < 6; col++){
            move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::SLOTS, row, col);
            context.wait_for_all_requests();
            // If no pokemon in the slot:
            if (!sth_in_box_detector.detect(env.console.video().snapshot())){
                
                // Move the to-keep pokemon in party to the empty slot.
                swap_two_box_slots(env.program_info(), env.console, context,
                    BoxCursorLocation::PARTY, pokemon_row_in_party, 0,
                    BoxCursorLocation::SLOTS, row, col);

                // Move back to middle box
                move_to_right_box(context);
                context.wait_for_all_requests();
                return true;
            }
        }
    }
    return false;
}

void EggAutonomous::reset_position_to_flying_spot(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    // Use map to fly back to the flying spot
    open_map_from_overworld(env.program_info(), env.console, context);
    pbf_move_left_joystick(context, 128, 160, 20, 50);
    fly_to_overworld_from_map(env.program_info(), env.console, context);
}

// From overworld, change pokemon party from the one used for getting eggs, to the one used for hatching
// The new party will be one flame body pokemon as lead, and some eggs.
// Function returns how many eggs are in the party
int EggAutonomous::picnic_party_to_hatch_party(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    env.console.overlay().add_log("Change to hatching", COLOR_WHITE);
    // change pokemon party from used for fetching eggs in picnic, to hatching on your ride.

    enter_box_system_from_overworld(env.program_info(), env.console, context);

    // Move to right box
    move_to_right_box(context);
    
    // Swap the stored the flame body pokemon with the stored first fetching pokemon
    swap_two_box_slots(env.program_info(), env.console, context,
        BoxCursorLocation::SLOTS, 0, 0,
        BoxCursorLocation::PARTY, 0, 0);

    // Unload rest of party to the 2nd column (col 1) in box
    unload_one_column_from_party(env.program_info(), env.console, context, 1);
    
    // Move to middle box
    move_to_left_box(context);

    // Load first egg column to party
    load_one_column_to_party(env.program_info(), env.console, context, 0);
    // Move cursor to party lead so that we can examine rest of party to detect eggs.
    move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::PARTY, 0, 0);

    const uint8_t num_eggs_in_party = check_only_eggs_in_party(env.program_info(), env.console, context);
    
    leave_box_system_to_overworld(env.program_info(), env.console, context);

    return num_eggs_in_party;
}


void EggAutonomous::save_game(SingleSwitchProgramEnvironment& env, BotBaseContext& context, bool from_overworld){
    try{
        if (from_overworld){
            save_game_from_overworld(env.program_info(), env.console, context);
        } else{
            save_game_from_menu(env.program_info(), env.console, context);
        }
        m_in_critical_to_save_stage = false;
    } catch(OperationFailedException &e){
        // To be safe: avoid interrupting or corrupting game saving,
        // make game saving non error recoverable
        throw FatalProgramException(env.logger(), e.message());
    }
}

void EggAutonomous::reset_game(SingleSwitchProgramEnvironment& env, BotBaseContext& context, const std::string& error_msg){
    try{
        pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
        context.wait_for_all_requests();
        reset_game_from_home(env, env.console, context, 5 * TICKS_PER_SECOND);
    } catch(OperationFailedException &e){
        // To be safe: avoid doing anything outside of game on Switch,
        // make game resetting non error recoverable
        throw FatalProgramException(env.logger(), e.message());
    }
}

void EggAutonomous::handle_recoverable_error(
    SingleSwitchProgramEnvironment& env,
    BotBaseContext& context,
    OperationFailedException& e,
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
        throw e;
    }

    if (AUTO_SAVING == AutoSave::AfterStartAndKeep && m_in_critical_to_save_stage){
        // We have found a pokemon to keep, but before we can save the game to protect the pokemon, an error occurred.
        // To not lose the pokemon, don't reset.
        // Note: if AUTO_SAVING == AutoSave::EveryBatch, then we don't need to care about this critical stage, because
        // in this auto saving mode, every batch of eggs have been saved beforehand.
        env.log("Found an error before we can save the game to protect the newly kept pokemon.", COLOR_RED);
        env.log("Don't reset game to protect it.", COLOR_RED);
        throw e;
    }

    consecutive_failures++;
    if (consecutive_failures >= 3){
        throw OperationFailedException(env.console, "Failed 3 times in the row.");
    }

    env.log("Reset game to handle recoverable error");
    reset_game(env, context, "handling recoverable error");
}


}
}
}
