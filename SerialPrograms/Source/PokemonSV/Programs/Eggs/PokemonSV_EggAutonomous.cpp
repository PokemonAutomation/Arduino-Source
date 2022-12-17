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
#include "PokemonSV/Programs/Box/PokemonSV_BoxRoutines.h"
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
    m_error_recoverable = true;

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
        bool need_stop_program = false;
        while(true){
            size_t consecutive_failures = 0;
            try {
                need_stop_program = hatch_eggs_full_routine(env, context, num_party_eggs);
                break;
            } catch(OperationFailedException& e){
                handle_recoverable_error(env, context, e, consecutive_failures);
                // After resetting the game, we don't know how many eggs in party
                num_party_eggs = -1;

                // If there is no save during egg hatching, then the game is reset to before fetching eggs
                // So we need to break out of the recoverable hatch egg routine loop
                if (m_saved_after_fetched_eggs == false){
                    break;
                }
            } // end try catch
        } // end recoverable loop to hatch eggs

        if (need_stop_program){
            break;
        }

        // If the program can't save-reload, then we cannot reload spent sandwich ingredients
        if (AUTO_SAVING == AutoSave::NoAutoSave){
            m_num_sandwich_spent++;
        } else if (m_saved_after_fetched_eggs){
            // If we save after fetching eggs, then the save solidifies spent sandwich ingredients.
            m_num_sandwich_spent++;
            m_saved_after_fetched_eggs = false;
        } else {
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
    try{
        picnic_at_zero_gate(env.console, context);
    } catch(OperationFailedException &e){
        dump_image_and_throw_recoverable_exception(env, env.console, NOTIFICATION_ERROR_RECOVERABLE,
            "PicnicFailed", e.message());
    }
    // Now we are at picnic. We are at one end of picnic table while the egg basket is at the other end
    
    bool can_make_sandwich = 0;
    
    try{
        can_make_sandwich = eat_egg_sandwich_at_picnic(env.realtime_dispatcher(), env.console, context);
    } catch(OperationFailedException &e){
        dump_image_and_throw_recoverable_exception(env, env.console, NOTIFICATION_ERROR_RECOVERABLE,
            "SandwichFailed", e.message());
    }
    if (can_make_sandwich == false){
        dump_unrecoverable_error(env, "NoRecipeOrIngredients");
        throw OperationFailedException(env.console, "No sandwich recipe or ingredients. Cannot open and select the sandwich recipe.");
    }
    env.current_stats<EggAutonomous_Descriptor::Stats>().m_sandwiches++;
    env.update_stats();

    // move past the table and collect eggs
    collect_eggs_at_picnic(env, context);

    try{
        leave_picnic(env.console, context);
    } catch(OperationFailedException &e){
        dump_image_and_throw_recoverable_exception(env, env.console, NOTIFICATION_ERROR_RECOVERABLE,
            "FailLeavePicnic", e.message());
    }

    // Reset position to flying spot:
    reset_position_to_flying_spot(env, context);

    return picnic_party_to_hatch_party(env, context);
}

bool EggAutonomous::hatch_eggs_full_routine(SingleSwitchProgramEnvironment& env, BotBaseContext& context, int num_eggs_in_party){    
    auto& stats = env.current_stats<EggAutonomous_Descriptor::Stats>();
    if (num_eggs_in_party < 0){
        // detect how many eggs in party
        enter_box_system(env, context);
    
        context.wait_for_all_requests();
        context.wait_for(std::chrono::seconds(1)); // wait for one second to let box UI fully loaded

        num_eggs_in_party = read_party_eggs(env, context);
        env.log("Read " + std::to_string(num_eggs_in_party) + " eggs.");
        env.console.overlay().add_log("Party eggs: " + std::to_string(num_eggs_in_party), COLOR_WHITE);

        leave_box_system(env, context);
    }

    bool need_to_save_after_kept = false;
    uint8_t next_egg_column = 0; // next egg column in box
    // The loop to hatch batches of eggs.
    // Each batch consists of at most five eggs.
    // There are at most six batches of eggs in a box.
    while(true){
        if (AUTO_SAVING == AutoSave::EveryBatch || (AUTO_SAVING == AutoSave::AfterStartAndKeep && need_to_save_after_kept)){
            save_game(env, context, true);
            m_saved_after_fetched_eggs = true;
            need_to_save_after_kept = false;
        }
        
        move_circles_to_hatch_eggs(env, context, num_eggs_in_party);

        enter_box_system(env, context);
        
        context.wait_for_all_requests();
        context.wait_for(std::chrono::seconds(1)); // wait for one second to let box UI fully loaded

        // Move left onto party lead
        pbf_press_dpad(context, DPAD_LEFT, 10, 40);
        // Move to 2nd pokemon in the party
        pbf_press_dpad(context, DPAD_DOWN, 10, 40);
        pbf_wait(context, 125);
        context.wait_for_all_requests();

        BoxDetector box_detector;
        SomethingInBoxSlotDetector sth_in_box_detector(COLOR_RED);
        for(int i = 0; i < num_eggs_in_party; i++){
            context.wait_for_all_requests();

            env.log("Check hatched pokemon at party slot " + std::to_string(i+1));
            
            bool found_shiny = false;
            auto shiny_callback = [&](bool shiny, const PokemonAutomation::ImageViewRGB32& screen){
                if (shiny){
                    found_shiny = true;
                    env.console.log("Shiny found!");
                    env.console.overlay().add_log("Shiny " + std::to_string(i+1) + "/" + std::to_string(num_eggs_in_party), COLOR_GREEN);        
                    stats.m_shinies++;
                    env.update_stats();
                    send_encounter_notification(
                        env,
                        m_notification_noop,
                        NOTIFICATION_SHINY,
                        false, true, {{{}, ShinyType::UNKNOWN_SHINY}}, std::nan(""),
                        screen
                    );
                } else{
                    env.console.overlay().add_log("Not shiny " + std::to_string(i+1) + "/" + std::to_string(num_eggs_in_party), COLOR_WHITE);
                }
            };

            EggHatchAction action = EggHatchAction::Release;
            check_baby_info(env.console, context, LANGUAGE, FILTERS, shiny_callback, action);

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

            bool found_empty_slot = false;
            switch (action){
                case EggHatchAction::StopProgram:
                    env.log("Program stop requested...");
                    env.console.overlay().add_log("Request program stop", COLOR_WHITE);
                    send_keep_notification();
                    return true;
                case EggHatchAction::Keep:
                    need_to_save_after_kept = true;
                    env.log("Moving Pokemon to keep box...", COLOR_BLUE);
                    stats.m_kept++;
                    env.update_stats();
                    m_num_kept++;
                    env.console.overlay().add_log("Keep pokemon " + std::to_string(m_num_kept) + "/" + std::to_string(MAX_KEEPERS), COLOR_YELLOW);
                    send_keep_notification();

                    // Move to left box
                    pbf_press_button(context, BUTTON_L, 20, 40);
                    for (uint8_t row = 0; row < 5; row++){
                        for (uint8_t col = 0; col < 6; col++){
                            move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::SLOTS, row, col);
                            context.wait_for_all_requests();
                            // If no pokemon in the slot:
                            if (!sth_in_box_detector.detect(env.console.video().snapshot())){
                                move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::PARTY, 1, 0);
                                // Hold the pokemon to keep
                                pbf_press_button(context, BUTTON_Y, 20, 40);
                                // Move the pokemon to the empty spot
                                move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::SLOTS, row, col);
                                context.wait_for_all_requests();

                                // Press A to drop the held pokemon to the empty slot
                                pbf_press_button(context, BUTTON_A, 20, 80);
                                // Move cursor back to party
                                move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::PARTY, 1, 0);
                                found_empty_slot = true;
                                break;
                            }
                        }
                        if (found_empty_slot){
                            break;
                        }
                    }
                    // Move back to middle box
                    pbf_press_button(context, BUTTON_R, 20, 40);

                    if (found_empty_slot == false){
                        env.log("No empty slot availble to place new pokemon.");
                        env.console.overlay().add_log("No box space", COLOR_RED);
                        return true;
                    }

                    if (m_num_kept >= MAX_KEEPERS){
                        env.log("Max keepers reached. Stopping program...");
                        env.console.overlay().add_log("Max Keepers reached", COLOR_WHITE);
                        return true;
                    }
                    break;
                
                case EggHatchAction::Release:
                default:
                    {
                        // Because game will fill hole in party automatically, so we don't want to 
                        // detect a hole in the pary:
                        const bool ensure_slot_empty = false;
                        release_one_pokemon(env.program_info(), env.console, context, ensure_slot_empty);
                    }
                    break;
            } // end switch EggHatchAction
        } // end for each hatched pokemon in party

        // Get the next egg column
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

            hold_one_column(context);
            // Move the new column to party
            move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::PARTY, 1, 0);
            // Drop the column to pary
            pbf_press_button(context, BUTTON_A, 20, 80);

            num_eggs_in_party = read_party_eggs(env, context);
        } else {
            // no more eggs in box, change to hatching mode:
            env.log("Replace party with picnic team");
            env.console.overlay().add_log("Change to picnic pokemon", COLOR_WHITE);
            
            // Move to right box
            pbf_press_button(context, BUTTON_R, 20, 40);
            // Move to party lead, the flame body pokemon
            move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::PARTY, 0, 0);
            // Hold the flame body pokemon at slot row 0, col 0
            pbf_press_button(context, BUTTON_Y, 20, 40);
            // Move to box row 0 col 0
            move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::SLOTS, 0, 0);
            // Swap lead with this one
            pbf_press_button(context, BUTTON_A, 20, 40);
            // Move to box row 0 col 1
            move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::SLOTS, 0, 1);

            hold_one_column(context);
            // Move to party
            move_box_cursor(env.program_info(), env.console, context, BoxCursorLocation::PARTY, 1, 0);
            // Drop rest of the party
            pbf_press_button(context, BUTTON_A, 20, 80);
            // Move back to middle box
            pbf_press_button(context, BUTTON_L, 20, 40);
        }

        leave_box_system(env, context);

        if (next_egg_column == 6){ // no more eggs to hatch
            if ((AUTO_SAVING == AutoSave::AfterStartAndKeep && need_to_save_after_kept) || AUTO_SAVING == AutoSave::EveryBatch){
                save_game(env, context, true);
                m_saved_after_fetched_eggs = true;
            }
            break; // break egg batch loop. This is the only place to break out of the loop
        }
        next_egg_column++;
    } // end egg batch loop
    
    return false;
}

void EggAutonomous::reset_position_to_flying_spot(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    // Use map to fly back to the flying spot
    open_map(env, context);
    pbf_move_left_joystick(context, 128, 160, 20, 50);
    fly_to_overworld(env, context);
}

// From overworld, change pokemon party from the one used for getting eggs, to the one used for hatching
// The new party will be one flame body pokemon as lead, and some eggs.
// Function returns how many eggs are in the party
int EggAutonomous::picnic_party_to_hatch_party(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    env.console.overlay().add_log("Change to hatching", COLOR_WHITE);
    // change pokemon party from used for fetching eggs in picnic, to hatching on your ride.

    enter_box_system(env, context);

    // Move to right box
    pbf_press_button(context, BUTTON_R, 20, 40);
    // Hold the flame body pokemon at slot row 0, col 0
    pbf_press_button(context, BUTTON_Y, 20, 40);
    // Move left onto party lead
    pbf_press_dpad(context, DPAD_LEFT, 10, 40);
    // Replace party lead with flame body hatcher pokemon
    pbf_press_button(context, BUTTON_A, 20, 40);
    // Move to 2nd pokemon in the party
    pbf_press_dpad(context, DPAD_DOWN, 10, 40);
    
    hold_one_column(context);

    // Move cursor to slot row 0 col 1
    pbf_press_dpad(context, DPAD_RIGHT, 10, 40);
    pbf_press_dpad(context, DPAD_RIGHT, 10, 40);
    pbf_press_dpad(context, DPAD_UP, 10, 40);
    // Drop the rest of party to second column in box
    pbf_press_button(context, BUTTON_A, 20, 40);
    // Back cursor to first slot in box
    pbf_press_dpad(context, DPAD_LEFT, 10, 40);

    // Move to middle box
    pbf_press_button(context, BUTTON_L, 20, 40);

    hold_one_column(context);

    // place column into pary
    pbf_press_dpad(context, DPAD_LEFT, 10, 40);
    pbf_press_dpad(context, DPAD_DOWN, 10, 40);
    pbf_press_button(context, BUTTON_A, 20, 40);

    context.wait_for_all_requests();
    context.wait_for(std::chrono::milliseconds(500)); // wait for one second to let box UI fully loaded

    int num_eggs_in_party = read_party_eggs(env, context);

    leave_box_system(env, context);

    return num_eggs_in_party;
}

void EggAutonomous::move_circles_to_hatch_eggs(SingleSwitchProgramEnvironment& env, BotBaseContext& context, int num_eggs_in_party){
    auto& stats = env.current_stats<EggAutonomous_Descriptor::Stats>();

    auto hatched_callback = [&](uint8_t){  
        stats.m_hatched++;
        env.update_stats();
    };
    try{
        hatch_eggs_at_zero_gate(env.console, context, (uint8_t)num_eggs_in_party, hatched_callback);
    } catch(OperationFailedException &e){
        dump_image_and_throw_recoverable_exception(env, env.console, NOTIFICATION_ERROR_RECOVERABLE,
            "HatchEgg", e.message());
    }

    reset_position_to_flying_spot(env, context);
}

uint8_t EggAutonomous::read_party_eggs(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    try{
        return check_egg_party_column(env.console, context).first;
    } catch(OperationFailedException &e){
        dump_image_and_throw_recoverable_exception(env, env.console, NOTIFICATION_ERROR_RECOVERABLE,
            "CantReadPartyEggs", e.message());
    }
}

// from one end of the picnic table, go around the table to reach basket and collect eggs
void EggAutonomous::collect_eggs_at_picnic(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    // Now we are at picnic. We are at one end of picnic table while the egg basket is at the other end
    auto& stats = env.current_stats<EggAutonomous_Descriptor::Stats>();
    auto basket_check_callback = [&](size_t new_eggs){
        stats.m_fetch_attempts++;
        stats.m_eggs += new_eggs;
        env.update_stats();
    };

    const size_t max_eggs = 30;
    size_t num_eggs_collected = 0;
    try{
        collect_eggs_after_sandwich(env.console, context, max_eggs, num_eggs_collected, basket_check_callback);
    } catch(OperationFailedException &e){
        dump_image_and_throw_recoverable_exception(env, env.console, NOTIFICATION_ERROR_RECOVERABLE,
            "CheckBasketFailed", e.message());
    }
}

void EggAutonomous::open_map(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    // Open map to fly to flying spot
    try{
        open_map_from_overworld(env.console, context);
    } catch(OperationFailedException &e){
        dump_image_and_throw_recoverable_exception(env, env.console, NOTIFICATION_ERROR_RECOVERABLE,
            "FailOpenMap", e.message());
    }
}

void EggAutonomous::fly_to_overworld(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    try{
        fly_to_overworld_from_map(env.console, context);
    } catch(OperationFailedException &e){
        dump_image_and_throw_recoverable_exception(env, env.console, NOTIFICATION_ERROR_RECOVERABLE,
            "FailFlyToOverworld", e.message());
    }
}

void EggAutonomous::enter_box_system(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    try{
        enter_box_system_from_overworld(env.console, context);
    } catch(OperationFailedException &e){
        dump_image_and_throw_recoverable_exception(env, env.console, NOTIFICATION_ERROR_RECOVERABLE,
            "FailEnterBoxSystem", e.message());
    }

}

void EggAutonomous::leave_box_system(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    try{
        leave_box_system_to_overworld(env.console, context);
    } catch(OperationFailedException &e){
        dump_image_and_throw_recoverable_exception(env, env.console, NOTIFICATION_ERROR_RECOVERABLE,
            "FailLeaveBoxSystem", e.message());
    }
}


void EggAutonomous::save_game(SingleSwitchProgramEnvironment& env, BotBaseContext& context, bool from_overworld){
    try{
        if (from_overworld){
            save_game_from_overworld(env.console, context);
        } else{
            save_game_from_menu(env.console, context);
        }
    } catch(OperationFailedException &e){
        // To be safe: avoid interrupting or corrupting game saving,
        // make game saving non error recoverable
        dump_unrecoverable_error(env, "FailSaveGame");
        throw e;
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
        dump_unrecoverable_error(env, "FailResetGame");
        throw e;
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
    // If error is not recoverable, or if there is no auto save, then we shouldn't reset game to
    // lose previous progress.
    if (m_error_recoverable == false || AUTO_SAVING == AutoSave::NoAutoSave){
        throw e;
    }

    consecutive_failures++;
    if (consecutive_failures >= 3){
        throw OperationFailedException(env.console, "Failed 3 times in the row.");
    }

    env.log("Reset game to handle recoverable error");
    reset_game(env, context, "handling recoverable error");
}

void EggAutonomous::dump_unrecoverable_error(SingleSwitchProgramEnvironment& env, const std::string& error_name){
    m_error_recoverable = false;
    env.console.overlay().add_log("Error: " + error_name, COLOR_RED);
    std::shared_ptr<const ImageRGB32> screen = env.console.video().snapshot();
    dump_image(env.console, env.program_info(), error_name, *screen);
}

}
}
}
