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
        : encounters(m_stats["Encounters"])
        , item_checks(m_stats["Item Checks"])
        , healing_trips(m_stats["Healing Trips"])
        , shinies(m_stats["Shinies"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Encounters");
        m_display_order.emplace_back("Item Checks");
        m_display_order.emplace_back("Healing Trips");
        m_display_order.emplace_back("Shinies", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& encounters;
    std::atomic<uint64_t>& item_checks;
    std::atomic<uint64_t>& healing_trips;
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
    , MAX_ENCOUNTERS(
        "<b>Max Encounters:</b><br>Set to 0 to continue indefinitely.",
        LockMode::UNLOCK_WHILE_RUNNING,
        0, 0 // default, min
    )
    , BATTLES_PER_ITEM_CHECK(
        "<b>Number of battles between item checks:</b><br>",
        LockMode::LOCK_WHILE_RUNNING,
        10, 1 // default, min
    )
    , MOVE_PP(
        "<b>PP of your lead " + Pokemon::STRING_POKEMON + "'s first move:</b><br>",
        LockMode::LOCK_WHILE_RUNNING,
        20, 5, 50 // default, min, max
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
    PA_ADD_OPTION(MAX_ENCOUNTERS);
    PA_ADD_OPTION(BATTLES_PER_ITEM_CHECK);
    PA_ADD_OPTION(MOVE_PP);
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

int grass_spin(SingleSwitchProgramEnvironment& env, ProControllerContext& context, bool leftright){
    // "walk" without moving by tapping the joystick to change directions
    // alternate between left/right and up/down to ensure there is always a direction change

    BlackScreenWatcher battle_entered(COLOR_RED);

    context.wait_for_all_requests();
    env.log("Starting grass spin.");
    WallClock deadline = current_time() + 60s;

    int ret = run_until<ProControllerContext>(
        env.console, context,
        [leftright, deadline](ProControllerContext& context) {
            while (current_time() < deadline){
                if (leftright){
                    pbf_move_left_joystick(context, {+1, 0}, 33ms, 150ms);
                    pbf_move_left_joystick(context, {-1, 0}, 33ms, 150ms);
                }else{
                    pbf_move_left_joystick(context, {0, +1}, 33ms, 150ms);
                    pbf_move_left_joystick(context, {0, -1}, 33ms, 150ms);
                }
            }
        },
        { battle_entered }
    );
    
    if (ret < 0){
        return -1;
    }

    bool encounter_shiny = handle_encounter(env.console, context, true);
    return encounter_shiny ? 1 : 0;
}

void use_first_battle_move(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    uint16_t errors = 0;
    while (true){    
        if (errors > 5) {
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to detect battle menu.",
                env.console
            );
        }

        BattleMenuWatcher menu_open(COLOR_RED);
        
        context.wait_for_all_requests();
        int ret = wait_until(
            env.console, context, 5000ms,
            { menu_open }
        );

        if (ret < 0) {
            env.log("Failed to detect battle menu within 5 seconds.");
            errors++;
            // attempt to return to the top-level battle menu
            pbf_mash_button(context, BUTTON_B, 2000ms);
            continue;
        }

        // mash A to use the move in the first position
        pbf_mash_button(context, BUTTON_A, 1000ms); 
        context.wait_for_all_requests();
        env.log("Used first move.");
        return;
    }
}

void take_pickup_items(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    env.log("Collecting items from party.");
    open_party_menu_from_overworld(env.console, context);
    pbf_move_left_joystick(context, {+1, 0}, 200ms, 300ms); // move to slot 2

    PartySelectionWatcher selection_open(COLOR_RED);
    int ret;
    // take items from positions 2-5. This works even if they haven't picked up an item
    for(int i = 2; i <= 5; i++) {
        context.wait_for_all_requests();
        ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context) {
                pbf_press_button(context, BUTTON_A, 200ms, 1800ms);
            },
            { selection_open }
        );
        if (ret < 0) {
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
    uint16_t moves_used = 0;
    uint16_t encounters_since_item_check = 0;
    
    while (!shiny_found){
        try{
            if (stats.encounters == 0 || failed_encounter || moves_used >= MOVE_PP){
                use_teleport_from_overworld(env.console, context);
                enter_pokecenter(env.console, context);
                heal_at_pokecenter(env.console, context);
                leave_pokecenter(env.console, context);
                stats.healing_trips++;
                if (GAME_LOCATION == GameLocation::route1){
                    walk_to_route1(env, context);
                }else if (GAME_LOCATION == GameLocation::route22){
                    walk_to_route22(env, context);
                }else{
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Option not yet implemented.",
                        env.console
                    );
                }
                moves_used = 0;
                failed_encounter = false;
            }

            uint16_t errors = 0;
            int ret = grass_spin(env, context, spin_leftright);
            shiny_found = (ret == 1);
            if (ret < 0){
                failed_encounter = true;
                env.log("Failed to trigger encounter: teleporting back to PokeCenter");
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
                encounters_since_item_check++;
                stats.encounters++;
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

            use_first_battle_move(env, context);
            moves_used++;
            bool move_learned = exit_wild_battle(env.console, context, !!STOP_ON_MOVE_LEARN, false);

            if (move_learned && STOP_ON_MOVE_LEARN){
                send_program_status_notification(
                    env, NOTIFICATION_STATUS_UPDATE,
                    "Stopping: move learned."
                );
                break;
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
