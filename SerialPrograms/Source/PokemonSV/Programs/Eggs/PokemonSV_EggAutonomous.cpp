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
#include "PokemonSV/Inference/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_MapDetector.h"
#include "PokemonSV/Inference/PokemonSV_OverworldDetector.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Programs/Eggs/PokemonSV_EggRoutines.h"
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
        : m_meals(m_stats["Meals"])
        , m_fetch_attempts(m_stats["Fetch Attempts"])
        , m_eggs(m_stats["Eggs"])
        , m_hatched(m_stats["Hatched"])
        , m_shinies(m_stats["Shinies"])
        , m_kept(m_stats[STRING_POKEMON + " Kept"])
        , m_errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Meals");
        m_display_order.emplace_back("Fetch Attempts");
        m_display_order.emplace_back("Eggs");
        m_display_order.emplace_back("Hatched");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back(STRING_POKEMON + " Kept");
        m_display_order.emplace_back("Errors", true);
    }

    std::atomic<uint64_t>& m_meals;
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
    , BUDGET(
        "<b>Budget:</b><br>How much " + STRING_POKEMON + " money to use on buying meals. One meal costs 2800.",
        LockWhileRunning::LOCKED,
        28000, 0
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
        "No auto-saving: No error/crash recovery. No money permanently spent.<br>"
        "Save at beginning and after keeping a baby: Allows for error/crash recovery. Money permanently spent after baby kept.<br>"
        "Save before every batch: Allows you to unhatch eggs. Money permanently spent after every meal.<br><br>"
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
    , MAX_NUM_BOXES(
        "<b>Max Num Boxes:</b><br>Maximum boxes of eggs to collect in picnic.<br>"
        "Prepare this many empty boxes, beginning at the current box when starting the program.",
        LockWhileRunning::LOCKED,
        1, 1, 30
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
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(BUDGET);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(MAX_KEEPERS);
    PA_ADD_OPTION(MAX_NUM_BOXES);
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
        collect_eggs_at_pokemon_league(env, context);
        // reset_game(env, context, "reset");
        return;
    }
    
    if (AUTO_SAVING == AutoSave::AfterStartAndKeep || AUTO_SAVING == AutoSave::EveryBatch){
        save_game(env, context, true);
        m_save_location_at_restaurant_town = true;
    }

    const size_t meal_cost = 2800;

    m_money_spent = 0;
    m_num_kept = 0;
    m_error_recoverable = true;

    while(true){
        // If in a previous egg auto iteration, we save at the hatching place (pokemon league),
        // we need to move the save back to the restaurant town:
        if (AUTO_SAVING != AutoSave::NoAutoSave && m_save_location_at_restaurant_town == false){
            save_game(env, context, true);
            m_save_location_at_restaurant_town = true;
        }

        // Do one iteration of the outmost loop of egg auto:
        // start at Cascarrafa (West) Pokecenter
        // go to Gastronome En Famille to buy meal
        // go out of town to collect eggs at picnic
        // go to Pokemon League to hatch eggs
        // go back to Cascarrafa (West).

        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        uint8_t num_newly_kept = 0;
        
        // Recoverable loop to fetch eggs:
        while(true){
            size_t consecutive_failures = 0;
            try {
                fetch_eggs_full_routine(env, context);
            } catch(OperationFailedException& e){
                handle_recoverable_error(env, context, e, consecutive_failures);
            } // end try catch
        } // end recoverable loop to fetch eggs:

        // XXX  debug break here to only test restaurant part
        break;

        // Recoverable loop to hatch eggs
        while(true){
            size_t consecutive_failures = 0;
            try {
                num_newly_kept = (uint8_t)hatch_eggs_full_routine(env, context);
            } catch(OperationFailedException& e){
                handle_recoverable_error(env, context, e, consecutive_failures);

                // If there is no save during egg hatching, then the game is reset to before going to restaurant
                // So we need to break out of the recoverable hatch egg routine loop
                if (m_save_location_at_restaurant_town){
                    break;
                }
            } // end try catch
        } // end recoverable loop to hatch eggs

        if (m_num_kept >= MAX_KEEPERS){
            env.log("End problem due to max keepers reached.");
            break;
        }

        // If we save at hatching location, then the save solidifies spent meal money
        if (m_save_location_at_restaurant_town == false){
            m_money_spent += meal_cost;
        }

        if (num_newly_kept == 0 && m_save_location_at_restaurant_town){
            // Nothing found in this iteration
            env.log("Resetting game since nothing found, saving restaurant meal cost.");

            reset_game(env, context, "reset to start new meal");
        }

        if (m_money_spent + meal_cost > BUDGET){
            env.console.overlay().add_log("Not enough money", COLOR_PURPLE);
            env.log("Not enough money to buy egg power, spent " + std::to_string(m_money_spent)
                + " budget " + std::to_string(BUDGET));
            break;
        }

        break; // TODO debug code here to prevent loop
        // end of one full meal->picnic->hatch iteration
    } // end the full egg autonomous loop

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

// The largest loop of egg auto:
// start at Cascarrafa (West) Pokecenter, go to Gastronome En Famille to buy meal, go out of town to collect
// eggs at picnic, go to Pokemon League to hatch eggs.
void EggAutonomous::fetch_eggs_full_routine(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    // Now the player character is at Cascarrafa (West) Pokecenter, go to restaurant
    from_pokecenter_get_meal(env, context);

    // Use map to fly to pokemon league
    open_map(env, context);
    pbf_press_button(context, BUTTON_ZL, 30, 100);
    pbf_move_left_joystick(context, 230, 255, 90, 50);
    fly_to_overworld(env, context);

    collect_eggs_at_pokemon_league(env, context);
}

size_t EggAutonomous::hatch_eggs_full_routine(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    if (AUTO_SAVING == AutoSave::EveryBatch){
        save_game(env, context, true);
        m_save_location_at_restaurant_town = false;
    }

    // routine should end at restaurant town
    return 0;
}

void EggAutonomous::from_pokecenter_get_meal(SingleSwitchProgramEnvironment& env, BotBaseContext& context){

    const size_t max_restaurant_tries = 5;

    for(size_t i_tries = 0; i_tries < max_restaurant_tries; i_tries++){
        env.console.overlay().add_log("Go to restaurant", COLOR_WHITE);
        env.log("Go to restaurant");
        // Orient camera to look at same direction as player character
        // This is needed because when save-load the game, the camera is reset
        // to this location.
        pbf_press_button(context, BUTTON_L, 50, 70);

        // Ride on Koraidon/Miradon
        pbf_press_button(context, BUTTON_PLUS, 50, 100);
        // Move right to make player character facing the road leading to Cascarrafa
        pbf_move_left_joystick(context, 255, 128, 50, 50);
        // Press L to move camera to face the same direction as the player character
        pbf_press_button(context, BUTTON_L, 50, 70);
        // Move forward
        pbf_move_left_joystick(context, 128, 0, 210, 0); // 250

        // Move right to make player character facing Cascarrafa
        pbf_move_left_joystick(context, 255, 110, 50, 50);
        // Press L to move camera to face the same direction as the player character
        pbf_press_button(context, BUTTON_L, 50, 70);
        // Move forward to enter Cascarrafa
        pbf_move_left_joystick(context, 128, 0, 300, 0); // 350

        // Move left to make player character facing the road leading to the restaurant
        pbf_move_left_joystick(context, 0, 100, 50, 50);
        // Press L to move camera to face the same direction as the player character
        pbf_press_button(context, BUTTON_L, 50, 70);
        // Move forward to go on the street that leads to restaurant
        pbf_move_left_joystick(context, 128, 0, 210, 0);

        context.wait_for_all_requests();

        int ret = 0;

        {
            AdvanceDialogWatcher dialog_watcher(COLOR_RED, std::chrono::milliseconds(100));        
            ret = run_until(
                env.console, context,
                [](BotBaseContext& context){
                    // Move right to make player character at front door of the restaurant
                    pbf_move_left_joystick(context, 255, 100, 150, 50);
                    // Enter restaurant
                    pbf_move_left_joystick(context, 128, 0, 125, 0);

                    pbf_wait(context, 600);
                },
                {{dialog_watcher}}
            );
        }

        if (ret < 0){
            // Fail to enter restaurant
            env.console.overlay().add_log("Restaurant not found", COLOR_PURPLE);
            env.log("Failed to enter restaurant, reset sequence");
            
            // Use map to reset to pokecenter
            open_map(env, context);
            pbf_move_left_joystick(context, 110, 250, 15, 50);
            fly_to_overworld(env, context);
        }
        else{
            env.console.overlay().add_log("Enter restaurant", COLOR_WHITE);
            env.log("Enter restaurant");

            // Eat food to gain egg power:
            try{
                order_compote_du_fils(env.console, context);
            } catch(OperationFailedException &e){
                dump_image_and_throw_recoverable_exception(env, env.console, NOTIFICATION_ERROR_RECOVERABLE,
                    "FailMeal", e.message());
            }
            
            context.wait_for_all_requests();   
            return;
        }
    } // end for going to restaurant loop

    dump_image_and_throw_recoverable_exception(env, env.console, NOTIFICATION_ERROR_RECOVERABLE,
        "CantGoToRestaurant", "Cannot go to restaurant from pokecenter after repeated tries.");
}

// From pokemon league pokecenter, go to the empty space in front of pokemon league to picnic and collect eggs
void EggAutonomous::collect_eggs_at_pokemon_league(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    // Move left to make player character facing the road leading to Cascarrafa
    pbf_move_left_joystick(context, 0, 128, 50, 50);
    // Press L to move camera to face the same direction as the player character
    pbf_press_button(context, BUTTON_L, 50, 70);
    // Move forward
    pbf_move_left_joystick(context, 128, 0, 250, 0);

    try{
        picnic_from_overworld(env.console, context);
    } catch(OperationFailedException &e){
        dump_image_and_throw_recoverable_exception(env, env.console, NOTIFICATION_ERROR_RECOVERABLE,
            "FailPicnic", e.message());
    }

    // Now we are at picnic. We are at one end of picnic table while the egg basket is at the other end
    // move past the table:

    context.wait_for_all_requests();
    env.log("Move past picnic table");
    env.console.overlay().add_log("Move past picnic table", COLOR_WHITE);

    // Move left
    pbf_move_left_joystick(context, 0, 128, 40, 40);
    // Move forward to pass table
    pbf_move_left_joystick(context, 128, 0, 100, 40);
    // Move right
    pbf_move_left_joystick(context, 255, 128, 40, 40);
    // Move back to face basket
    pbf_move_left_joystick(context, 128, 255, 10, 40);

    context.wait_for_all_requests();

    const size_t max_eggs = MAX_NUM_BOXES * 30;
    size_t num_eggs_collected = 0;

    
    const auto egg_collection_interval = std::chrono::minutes(3);
    const auto max_egg_wait_time = std::chrono::minutes(20);

    WallClock start = current_time();
    while(true){
        try{
            collect_eggs_from_basket(env.console, context, max_eggs, num_eggs_collected);
        } catch(OperationFailedException &e){
            dump_image_and_throw_recoverable_exception(env, env.console, NOTIFICATION_ERROR_RECOVERABLE,
                "FailEggBasket", e.message());
        }

        if (num_eggs_collected == max_eggs){
            env.log("Collected enough eggs.");
            break;
        }

        if (current_time() - start > max_egg_wait_time){
            env.log("Picnic time up.");
            env.console.overlay().add_log("Picnic tiem up", COLOR_YELLOW);
            break;
        }
        
        context.wait_for_all_requests();
        env.log("Wait 3 minutes.");
        env.console.overlay().add_log("Wait 3 min", COLOR_WHITE);
        context.wait_for(egg_collection_interval);
    }

    try{
        leave_picnic(env.console, context);
    } catch(OperationFailedException &e){
        dump_image_and_throw_recoverable_exception(env, env.console, NOTIFICATION_ERROR_RECOVERABLE,
            "FailLeavePicnic", e.message());
    }

}

void EggAutonomous::open_map(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    // Open map to fly back to PokeCenter
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
        m_error_recoverable = false;
        env.console.overlay().add_log("Error: FailSaveGame", COLOR_RED);
        std::shared_ptr<const ImageRGB32> screen = env.console.video().snapshot();
        dump_image(env.console, env.program_info(), "FailSaveGame", *screen);
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
        m_error_recoverable = false;
        env.console.overlay().add_log("Error: FailResetGame", COLOR_RED);
        std::shared_ptr<const ImageRGB32> screen = env.console.video().snapshot();
        dump_image(env.console, env.program_info(), "FailResetGame", *screen);
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

}
}
}
