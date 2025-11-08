/*  Shiny Hunt - Wild Zone Entrance
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA/Programs/PokemonLZA_GameEntry.h"
#include "PokemonLZA_WildZoneEntrance.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {

using namespace Pokemon;


// TODO:
// Tell user to set text speed to fast!


ShinyHunt_WildZoneEntrance_Descriptor::ShinyHunt_WildZoneEntrance_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:ShinyHunt-WildZoneEntrance", STRING_POKEMON + " LZA",
        "Shiny Hunt - Wild Zone Entrance",
        "Programs/PokemonLZA/ShinyHunt-WildZoneEntrance.html",
        "Shiny hunt by repeatedly entering Wild Zone from its entrance.",
        ProgramControllerClass::StandardController_NoRestrictions, FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS, {}
    )
{}
class ShinyHunt_WildZoneEntrance_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : visits(m_stats["Visits"])
        , chased(m_stats["Chased"])
        , shinies(m_stats["Shiny Sounds"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Visits");
        m_display_order.emplace_back("Chased");
        m_display_order.emplace_back("Shiny Sounds");
        m_display_order.emplace_back("Game Resets");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
        m_aliases["Wild Zone"] = "Visits";
    }

    std::atomic<uint64_t>& visits;
    std::atomic<uint64_t>& chased;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> ShinyHunt_WildZoneEntrance_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


ShinyHunt_WildZoneEntrance::ShinyHunt_WildZoneEntrance()
    : WALK_TIME_IN_ZONE(
        "<b>Walk in Zone:</b><br>Walk this long in the zone after passing through the gate.",
        LockMode::UNLOCK_WHILE_RUNNING,
        0ms, 10s,
        "500 ms"
    )
    , RUNNING(
        "<b>Running:</b><br>Running instead of walking while in the zone",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , SHINY_DETECTED("Shiny Detected", "", "2000 ms", ShinySoundDetectedAction::NOTIFY_ON_FIRST_ONLY)
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &SHINY_DETECTED.NOTIFICATIONS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_STATIC(SHINY_REQUIRES_AUDIO);
    PA_ADD_OPTION(WILD_ZONE);
    PA_ADD_OPTION(WALK_TIME_IN_ZONE);
    PA_ADD_OPTION(RUNNING);
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}

// After fast travel, move forward to enter wild zone.
// This function is robust against day/night changes
void go_to_entrance(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
){
    ButtonWatcher buttonA(COLOR_RED, ButtonType::ButtonA, {0.3, 0.2, 0.4, 0.7}, &env.console.overlay());
    run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            for (int c = 0; c < 30; c++){
                pbf_move_left_joystick(context, 128, 0, 800ms, 200ms);
            }
        },
        {{buttonA}}
    );
    env.console.overlay().add_log("Detect Entrance");
}


// Fast travel back to zone entrance.
// This is called by do_one_wild_zone_trip() at end of a trip, or by
// program() when a shiny sound is detected.
// The function handles day/night changes on its own.
// Note: day/night change can happen at any time, including when the player
// character is pressing A to enter the zone. When this happens, the button
// press is eaten and the character will be still outside the zone.
// leave_zone() also handles this case.
void leave_zone(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    Milliseconds walk_time_in_zone,
    WildZone wild_zone
){
    ShinyHunt_WildZoneEntrance_Descriptor::Stats& stats = env.current_stats<ShinyHunt_WildZoneEntrance_Descriptor::Stats>();

    FastTravelState travel_status = FastTravelState::PURSUED;
    bool can_fast_travel = open_map(env.console, context);
    // Open map is robust against day/night change. So after open_map()
    // we are sure we are in map view
    if (can_fast_travel){
        travel_status = fly_from_map(env.console, context);
    }
    if (travel_status == FastTravelState::SUCCESS){
        // we were in the zone and now successfully travel back to entrance
        env.log("Finish one trip");
        env.console.overlay().add_log("Finish One Trip");
        stats.visits++;
        env.update_stats();
        return;
    } else if (travel_status == FastTravelState::NOT_AT_FLY_SPOT){
        // we cannot fast travel at current location. This means we are actually outside
        // the wild zone!
        // Assume we are still outside the entrance, probably due to the mashing A to enter
        // zone button press was eaten by a day/night change.
        
        env.log("Not at fly spot now, probably still outside zone at entrance.");
        env.console.overlay().add_log("Still at Entrance");

        // From zone entrance, fast travel back to the zone entrance
        // to reset player character orientation
        move_map_cursor_from_entrance_to_zone(env.console, context, wild_zone);

        travel_status = fly_from_map(env.console, context);
        if (travel_status != FastTravelState::SUCCESS){
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "leave_zone(): Cannot fast travel to zone from outside the entrance.",
                env.console
            );
        }

        stats.visits++;
        env.update_stats();
        return;
    } 
    // travel_status == FastTravelState::PURSUED
    // we are being attacked by wild pokemon.
    
    // mash B to close map and return to overworld
    pbf_mash_button(context, BUTTON_B, 1600ms);
    walk_time_in_zone += 2s; // give some extra time
    env.log("Escaping");
    env.console.overlay().add_log("Escaping Back to Entrance");
    stats.chased++;
    env.update_stats();

    ButtonWatcher buttonA(COLOR_RED, ButtonType::ButtonA, {0.3, 0.2, 0.4, 0.7}, &env.console.overlay());
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&walk_time_in_zone](ProControllerContext& context){
            // running back
            ssf_press_button(context, BUTTON_B, 0ms, walk_time_in_zone, 0ms);
            pbf_move_left_joystick(context, 128, 255, walk_time_in_zone, 0ms);
        },
        {{buttonA}}
    );
    if (ret != 0){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "leave_zone(): Cannot run back to entrance after being chased by wild pokemon.",
            env.console
        );
    }
    // Found button A, so we are at the entrance.
    // Mash A to leave Zone.
    env.log("Found button A. Leaving Zone");
    env.console.overlay().add_log("Found Button A. Leaving Zone");
    pbf_mash_button(context, BUTTON_A, 2000ms);
    // From zone entrance, fast travel back to the zone entrance
    // to reset aggressive pokemon
    can_fast_travel = open_map(env.console, context);
    
    if (!can_fast_travel){
        stats.errors++;
        env.update_stats();

        // we should be outside zone but we cannot fast travel. Probably:
        // - The button A is actually from a nearby bench! We are lost
        // - The button A mashing sequence was eaten by day/night change
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "leave_zone(): Failed to mash A to exit zone despite finding Button A.",
            env.console
        );
    }
    
    move_map_cursor_from_entrance_to_zone(env.console, context, wild_zone);

    travel_status = fly_from_map(env.console, context);
    if (travel_status == FastTravelState::SUCCESS){
        stats.visits++;
        env.update_stats();
        return;
    }
    // we are outside the zone but cannot move to the zone fast travel icon
    stats.errors++;
    env.update_stats();
    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "leave_zone(): Cannot move to the fast travel icon of zone " + 
        std::to_string(int(wild_zone)+1) + " after leaving entrance",
        env.console
    );
}

// After fast travel back to a wild zone, go through entrance and move forward.
// Then open map to fast travel back to entrance.
// If encountered day/night change, wait till change is complete and fast travel back to entrance.
// If targeted by wild pokemon, run back to entrance and fast travel to reset spawns and aggro.
// The function handles day/night changes on its own.
void do_one_wild_zone_trip(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    Milliseconds walk_time_in_zone,
    bool running,
    WildZone wild_zone
){
    context.wait_for_all_requests();

    go_to_entrance(env, context);

    // Mash button A to enter the zone.
    pbf_mash_button(context, BUTTON_A, 2000ms);
    context.wait_for_all_requests();
    // Day/night change can happen before or after the button A mash, so we are not
    // sure if we are in the zone or not! But at end of the travel we will fast
    // travel back to entrance and have a way to work on both cases.
    // move forward
    if (walk_time_in_zone > Milliseconds::zero()){
        if (running){
            env.console.overlay().add_log("Running");
            ssf_press_button(context, BUTTON_B, 0ms, walk_time_in_zone, 0ms);
        } else{
            env.console.overlay().add_log("Walking");
        }
        pbf_move_left_joystick(context, 128, 0, walk_time_in_zone, 200ms);
    }
    context.wait_for_all_requests();
    leave_zone(env, context, walk_time_in_zone, wild_zone);
}

void ShinyHunt_WildZoneEntrance::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ShinyHunt_WildZoneEntrance_Descriptor::Stats& stats =
        env.current_stats<ShinyHunt_WildZoneEntrance_Descriptor::Stats>();

    if (SHINY_DETECTED.ACTION == ShinySoundDetectedAction::NOTIFY_ON_ALL){
        throw UserSetupError(
            env.console,
            "Shiny would be detected/notified at most once. Choose one of the other 2 options."
        );
    }

    bool leave_zone_first = false;
    while (true){
        float shiny_coefficient = 1.0;
        PokemonLA::ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool {
            //  Warning: This callback will be run from a different thread than this function.
            stats.shinies++;
            env.console.overlay().add_log("Shiny Sound Detected!", COLOR_YELLOW);
            env.update_stats();
            shiny_coefficient = error_coefficient;
            return true;
        });

        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                if (leave_zone_first){
                    leave_zone(env, context, WALK_TIME_IN_ZONE, WILD_ZONE);
                    leave_zone_first = false;
                }
                while (true){
                    do_one_wild_zone_trip(env, context, WALK_TIME_IN_ZONE, RUNNING, WILD_ZONE);
                    send_program_status_notification(env, NOTIFICATION_STATUS);
                }
            },
            {{shiny_detector}}
        );

        //  This should never happen.
        if (ret != 0){
            continue;
        }

        // heard shiny sound. Mash B to go back from any already opened menus
        pbf_mash_button(context, BUTTON_B, 1000ms);

        // decide whether to exit the program
        bool exit = SHINY_DETECTED.on_shiny_sound(
            env, env.console, context,
            stats.shinies,
            shiny_coefficient
        );
        if (exit){
            break;
        }
        leave_zone_first = true;
    }

    go_home(env.console, context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
