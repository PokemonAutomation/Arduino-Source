/*  Pickup Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Pokemon/Pokemon_Strings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_BattleDialogs.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_PartyMenuDetector.h"
#include "PokemonFRLG/Programs/PokemonFRLG_StartMenuNavigation.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_PickupFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

PickupFarmer_Descriptor::PickupFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:PickupFarmer",
        Pokemon::STRING_POKEMON + " FRLG", "Pickup Farmer",
        "Programs/PokemonFRLG/PickupFarmer.html",
        "Farms berries, nuggets, rare candies, PP-UP, and TM10 by battling wild encounters and periodically checking for picked-up items.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct PickupFarmer_Descriptor::Stats : public StatsTracker{
    Stats()
        : encounters(m_stats["Encounters Won"])
        , item_checks(m_stats["Item Checks"])
        , healing_trips(m_stats["Healing Trips"])
        , times_fainted(m_stats["Times Fainted"])
        , shinies(m_stats["Shinies"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Encounters Won");
        m_display_order.emplace_back("Item Checks");
        m_display_order.emplace_back("Healing Trips");
        m_display_order.emplace_back("Times Fainted", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Shinies", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& encounters;
    std::atomic<uint64_t>& item_checks;
    std::atomic<uint64_t>& healing_trips;
    std::atomic<uint64_t>& times_fainted;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> PickupFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

PickupFarmer::PickupFarmer()
    : GAME_LOCATION(
        "<b>Game Location:</b><br>",
        {
            {GameLocation::route1,  "route1",  "Route 1"},
            {GameLocation::route22, "route22", "Route 22"}
        },
        LockMode::LOCK_WHILE_RUNNING,
        GameLocation::route1
    )
    , TRAVEL_METHOD(
        "<b>Travel Method:</b><br>This move should be learned by the last " + Pokemon::STRING_POKEMON + " in your party.",
        {
            {TravelMethod::fly,  "fly",  "Fly"},
            {TravelMethod::teleport, "teleport", "Teleport"}
        },
        LockMode::LOCK_WHILE_RUNNING,
        TravelMethod::fly
    )
    , MAX_ENCOUNTERS(
        "<b>Max Encounters to Defeat:</b><br>Set to 0 to continue indefinitely.",
        LockMode::UNLOCK_WHILE_RUNNING,
        0, 0 // default, min
    )
    , BATTLES_PER_ITEM_CHECK(
        "<b>Number of battles between item checks:</b><br>",
        LockMode::LOCK_WHILE_RUNNING,
        10, 1 // default, min
    )
    , PREVENT_EVOLUTION(
        "<b>Prevent " + Pokemon::STRING_POKEMON + " from evolving</b>",
        LockMode::LOCK_WHILE_RUNNING, 
        false // default
    )
    , STOP_ON_MOVE_LEARN(
        "<b>Quit when a new move is learned</b><br>Stop this program when a new move is learned. If unchecked, new moves will not be learned.",
        LockMode::LOCK_WHILE_RUNNING, 
        false // default
    )
    , IGNORE_SHINIES(
        "<b>Ignore shinies</b><br>Do not stop the program when a wild shiny is encountered.",
        LockMode::LOCK_WHILE_RUNNING, 
        false // default
    )
    , TAKE_VIDEO(
        "<b>Take Video:</b><br>Record a video when the shiny is found.", 
        LockMode::UNLOCK_WHILE_RUNNING, 
        true // default
    )
    , GO_HOME_WHEN_DONE(true)
    , NOTIFICATION_SHINY(
        "Shiny found",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_SHINY,
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(GAME_LOCATION);
    PA_ADD_OPTION(TRAVEL_METHOD);
    PA_ADD_OPTION(MAX_ENCOUNTERS);
    PA_ADD_OPTION(BATTLES_PER_ITEM_CHECK);
    PA_ADD_OPTION(STOP_ON_MOVE_LEARN);
    PA_ADD_OPTION(IGNORE_SHINIES);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

namespace{

void walk_to_route1(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    env.log("Walking to Route 1.");
    // left a couple of steps
    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms);
    // down to the tall grass
    pbf_move_left_joystick(context, {0, -1}, 5200ms, 100ms);
    // left and up to the corner
    pbf_move_left_joystick(context, {-1, 0}, 900ms, 100ms);
    pbf_move_left_joystick(context, {0, +1}, 900ms, 900ms);
    context.wait_for_all_requests();
}

void walk_to_route22(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    env.log("Walking to Route 22.");
    // left a few steps
    pbf_move_left_joystick(context, {-1, 0}, 900ms, 200ms);
    // up to the bush
    pbf_move_left_joystick(context, {0, +1}, 2300ms, 200ms);
    // left to the trees
    pbf_move_left_joystick(context, {-1, 0}, 7800ms, 200ms);
    // down and over the ledge
    pbf_move_left_joystick(context, {0, -1}, 3000ms, 200ms);
    // left a couple of steps
    pbf_move_left_joystick(context, {-1, 0}, 600ms, 200ms);
    // up to into the grass
    pbf_move_left_joystick(context, {0, +1}, 1500ms, 500ms);
    context.wait_for_all_requests();
}

void take_pickup_items(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    env.log("Collecting items from party.");
    open_party_menu_from_overworld(env.console, context);
    pbf_move_left_joystick(context, {+1, 0}, 200ms, 300ms); // move to slot 2

    PartySelectionWatcher selection_open(COLOR_RED);
    int ret;
    // take items from positions 2-5. This works even if they haven't picked up an item
    for(int i = 2; i <= 5; i++){
        context.wait_for_all_requests();
        ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                pbf_press_button(context, BUTTON_A, 200ms, 1800ms);
            },
            { selection_open }
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to detect selection menu.",
                env.console
            );
        }
        pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
        pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
        pbf_press_button(context, BUTTON_A, 200ms, 300ms);
        pbf_move_left_joystick(context, {0, -1}, 200ms, 300ms);
        pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
        pbf_press_button(context, BUTTON_A, 200ms, 300ms);
        pbf_move_left_joystick(context, {0, -1}, 200ms, 300ms);
    }
    // close the start menu
    pbf_press_button(context, BUTTON_B, 200ms, 300ms);
    pbf_press_button(context, BUTTON_B, 200ms, 800ms);
    close_start_menu(env.console, context);
}

} // namespace


void PickupFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    /*
    * Settings: Text Speed fast
    */

    PickupFarmer_Descriptor::Stats& stats = env.current_stats<PickupFarmer_Descriptor::Stats>();

    home_black_border_check(env.console, context);

    bool shiny_found = false;
    bool failed_encounter = false;

    bool spin_leftright = true;
    bool out_of_pp = false;
    uint16_t encounters_since_item_check = 0;
    
    while (!shiny_found){
        try{
            if (stats.encounters == 0 || failed_encounter || out_of_pp){
                switch (TRAVEL_METHOD){
                case TravelMethod::fly:
                    open_fly_map_from_overworld(env.console, context);
                    fly_from_kanto_map(env.console, context, KantoFlyLocation::viridiancity);
                    break;
                case TravelMethod::teleport:
                    use_teleport_from_overworld(env.console, context);
                    break;
                default:
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Option not yet implemented.",
                    env.console
                );
                }
                enter_pokecenter(env.console, context);
                heal_at_pokecenter(env.console, context);
                leave_pokecenter(env.console, context);
                stats.healing_trips++;
                switch (GAME_LOCATION){
                case GameLocation::route1:
                    walk_to_route1(env, context);
                    break;
                case GameLocation::route22:
                    walk_to_route22(env, context);
                    break;
                default:
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Option not yet implemented.",
                        env.console
                    );
                }
                
                out_of_pp = false;
                failed_encounter = false;
                spin_leftright = true;
            }

            uint16_t errors = 0;
            int ret = grass_spin(env.console, context, spin_leftright);
            shiny_found = (ret == 1);
            if (ret < 0){
                failed_encounter = true;
                env.log("Failed to trigger encounter: traveling back to PokeCenter");
                errors++;
                if (errors >= 5){
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Failed 5 times to trigger a wild encounter within 60 seconds",
                        env.console
                    );
                }
                // exit a menu in case there is one open
                pbf_mash_button(context, BUTTON_B, 1000ms);
                continue;
            }else{
                spin_leftright = !spin_leftright;
            }

            if (shiny_found && !IGNORE_SHINIES){
                env.log("Shiny found!");
                stats.shinies++;
                VideoSnapshot screen = env.console.video().snapshot();
                send_program_notification(
                    env,
                    NOTIFICATION_SHINY,
                    COLOR_YELLOW,
                    "Shiny found!",
                    {}, "",
                    screen,
                    true
                );
                if (TAKE_VIDEO){
                    pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 0ms);
                }
                break;
            }
            shiny_found = false;

            int ret2 = spam_first_move(env.console, context);
            if (ret2 == 1) { // user fainted
                stats.times_fainted++;
                out_of_pp = true; // triggers a healing trip
                //TODO: handle exiting the battle in case the player can't escape
                pbf_mash_button(context, BUTTON_B, 5000ms);
                context.wait_for_all_requests();
            } else if (ret2 == 2){ // battle fled (no EV gain)
                // continue;
            } else if (ret2 == 3){
                out_of_pp = true;
            } else if (ret2 == 0){ // opponent fainted
                stats.encounters++;
                encounters_since_item_check++;
                bool move_learned = exit_wild_battle(env.console, context, !!STOP_ON_MOVE_LEARN, !!PREVENT_EVOLUTION);
                if (move_learned && STOP_ON_MOVE_LEARN){
                    send_program_status_notification(
                        env, NOTIFICATION_STATUS_UPDATE,
                        "Stopping: move learned."
                    );
                    break;
                }
            }

            if (encounters_since_item_check >= BATTLES_PER_ITEM_CHECK){
                take_pickup_items(env, context);
                stats.item_checks++;
                encounters_since_item_check = 0;
            }

            if (MAX_ENCOUNTERS > 0 && stats.encounters >= MAX_ENCOUNTERS){
                send_program_status_notification(
                    env, NOTIFICATION_STATUS_UPDATE,
                    "Maximum resets reached."
                );
                break;
            }else{
                send_program_status_notification(
                    env, NOTIFICATION_STATUS_UPDATE,
                    "Farming."
                );
                env.update_stats();
                context.wait_for_all_requests();
            }
        }catch (OperationFailedException&){
            stats.errors++;
            throw;
        }
    }

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}
