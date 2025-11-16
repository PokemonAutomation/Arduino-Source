/*  Shiny Hunt - Wild Zone Entrance
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_OverworldPartySelectionDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA/Programs/PokemonLZA_GameEntry.h"
#include "PokemonLZA_WildZoneEntrance.h"

// #include <iostream>
// using std::cout, std::endl;

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {

using namespace Pokemon;

WildZoneOption::WildZoneOption()
    : EnumDropdownOption<WildZone>(
            "<b>Wild Zone:</b>",
            {
                {WildZone::WILD_ZONE_1,  "wild-zone-1",  "Wild Zone 1"},
                {WildZone::WILD_ZONE_2,  "wild-zone-2",  "Wild Zone 2"},
                {WildZone::WILD_ZONE_3,  "wild-zone-3",  "Wild Zone 3"},
                {WildZone::WILD_ZONE_4,  "wild-zone-4",  "Wild Zone 4"},
                {WildZone::WILD_ZONE_5,  "wild-zone-5",  "Wild Zone 5"},
                {WildZone::WILD_ZONE_6,  "wild-zone-6",  "Wild Zone 6"},
                {WildZone::WILD_ZONE_7,  "wild-zone-7",  "Wild Zone 7"},
                {WildZone::WILD_ZONE_8,  "wild-zone-8",  "Wild Zone 8"},
                {WildZone::WILD_ZONE_9,  "wild-zone-9",  "Wild Zone 9"},
                {WildZone::WILD_ZONE_10, "wild-zone-10", "Wild Zone 10"},
                {WildZone::WILD_ZONE_11, "wild-zone-11", "Wild Zone 11"},
                {WildZone::WILD_ZONE_12, "wild-zone-12", "Wild Zone 12"},
                {WildZone::WILD_ZONE_13, "wild-zone-13", "Wild Zone 13"},
                {WildZone::WILD_ZONE_14, "wild-zone-14", "Wild Zone 14"},
                {WildZone::WILD_ZONE_15, "wild-zone-15", "Wild Zone 15"},
                {WildZone::WILD_ZONE_16, "wild-zone-16", "Wild Zone 16"},
                {WildZone::WILD_ZONE_17, "wild-zone-17", "Wild Zone 17"},
                {WildZone::WILD_ZONE_18, "wild-zone-18", "Wild Zone 18"},
                {WildZone::WILD_ZONE_19, "wild-zone-19", "Wild Zone 19"},
                {WildZone::WILD_ZONE_20, "wild-zone-20", "Wild Zone 20"},
            },
            LockMode::LOCK_WHILE_RUNNING,
            WildZone::WILD_ZONE_1
        )
    {}
        


ShinyHunt_WildZoneEntrance_Descriptor::ShinyHunt_WildZoneEntrance_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:ShinyHunt-WildZoneEntrance", STRING_POKEMON + " LZA",
        "Wild Zone Entrance",
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
        , game_resets(m_stats["Game Resets"])
        , errors(m_stats["Errors"])
        , day_changes(m_stats["Day/Night Changes"])
    {
        m_display_order.emplace_back("Visits");
        m_display_order.emplace_back("Chased", PreloadSettings::instance().DEVELOPER_MODE ? ALWAYS_VISIBLE : ALWAYS_HIDDEN);
        m_display_order.emplace_back("Shiny Sounds");
        m_display_order.emplace_back("Game Resets");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Day/Night Changes", ALWAYS_HIDDEN);
        m_aliases["Wild Zone"] = "Visits";
    }

    std::atomic<uint64_t>& visits;
    std::atomic<uint64_t>& chased;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& game_resets;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& day_changes;
};
std::unique_ptr<StatsTracker> ShinyHunt_WildZoneEntrance_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


ShinyHunt_WildZoneEntrance::ShinyHunt_WildZoneEntrance()
    : MOVEMENT(
        "<b>Movement:</b>",
        {
            {0, "no-movement", "No Movement"},
            {1, "approach-gate", "Approach Gate But Don't Enter"},
            {2, "enter-zone", "Enter Zone (DANGER! Avoid Wild " + Pokemon::STRING_POKEMON + " Noticing You!)"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        2
    )
    , WALK_TIME_IN_ZONE(
        "<b>Walk Time in Zone:</b><br>Walk this long in the zone after passing through the gate.",
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
    PA_ADD_OPTION(MOVEMENT);
    PA_ADD_OPTION(WALK_TIME_IN_ZONE);
    PA_ADD_OPTION(RUNNING);
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);

    MOVEMENT.add_listener(*this);
}

void ShinyHunt_WildZoneEntrance::on_config_value_changed(void* object){
    if (MOVEMENT.current_value() <= 1){
        // no entering zone, disable related options
        WALK_TIME_IN_ZONE.set_visibility(ConfigOptionState::DISABLED);
        RUNNING.set_visibility(ConfigOptionState::DISABLED);
    } else{
        // entering zone
        WALK_TIME_IN_ZONE.set_visibility(ConfigOptionState::ENABLED);
        RUNNING.set_visibility(ConfigOptionState::ENABLED);
    }
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
                ssf_press_button(context, BUTTON_B, 0ms, 2s, 0ms);
                pbf_move_left_joystick(context, 128, 0, 2s, 200ms);
            }
        },
        {{buttonA}}
    );
    env.console.overlay().add_log("Detect Entrance");
}


void fast_travel_outside_zone(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    WildZone wild_zone,
    bool to_max_zoom_level_on_map,
    std::string extra_error_msg = ""
){
    ShinyHunt_WildZoneEntrance_Descriptor::Stats& stats = env.current_stats<ShinyHunt_WildZoneEntrance_Descriptor::Stats>();

    bool can_fast_travel = open_map(env.console, context, to_max_zoom_level_on_map);
    if (!can_fast_travel){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "fast_travel_outside_zone(): Fast travel disabled from supposedly outside the entrance." + extra_error_msg,
            env.console
        );
    }

    move_map_cursor_from_entrance_to_zone(env.console, context, wild_zone);

    FastTravelState travel_status = fly_from_map(env.console, context);
    if (travel_status != FastTravelState::SUCCESS){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "fast_travel_outside_zone(): After moving map cursor, cannot fast travel to the zone." + extra_error_msg,
            env.console
        );
    }
}


// Go back to zone entrance by fast travel if possible or by running backwards
// if under attack by wild pokemon.
// After leaving the gate, do a fast travel to the current zone to reset spawns.
//
// This is called by do_one_wild_zone_trip() at end of a trip, or by
// program() when a shiny sound is detected.
//
// The function handles day/night changes on its own.
// TODO: day/night change handling is still under development
//
// Note: day/night change can happen at any time, including when the player
// character is pressing A to enter the zone. When this happens, the button
// press is eaten and the character will be still outside the zone.
// leave_zone_and_reset_spawns() also handles this case.
void leave_zone_and_reset_spawns(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    Milliseconds walk_time_in_zone,
    WildZone wild_zone,
    ShinySoundHandler& shiny_sound_handler,
    bool to_max_zoom_level_on_map
){
    ShinyHunt_WildZoneEntrance_Descriptor::Stats& stats = env.current_stats<ShinyHunt_WildZoneEntrance_Descriptor::Stats>();

    FastTravelState travel_status = FastTravelState::PURSUED;
    bool can_fast_travel = open_map(env.console, context, to_max_zoom_level_on_map);
    // Open map is robust against day/night change. So after open_map()
    // we are sure we are in map view
    if (can_fast_travel){
        travel_status = fly_from_map(env.console, context);
    }
    if (travel_status == FastTravelState::SUCCESS){
        // we were in the zone and now successfully travel back to entrance
        env.log("Leave zone successfully by fast travel");
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
                "leave_zone_and_reset_spawns(): Cannot fast travel to zone from outside the entrance.",
                env.console
            );
        }
        env.log("Fast travel after failed to enter zone by day/night change");
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
#if 0
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "leave_zone_and_reset_spawns(): Cannot run back to entrance after being chased by wild pokemon.",
            env.console
        );
#else
        throw UserSetupError(
            env.logger(),
            "This program requires that you do not get attacked. "
            "Please choose a location/route that is safe from attack."
        );
#endif
    }
    shiny_sound_handler.process_pending(context);

    // Found button A, so we are at the entrance.
    // Mash A to leave Zone.
    env.log("Found button A. Leaving Zone");
    env.console.overlay().add_log("Found Button A. Leaving Zone");
    pbf_mash_button(context, BUTTON_A, 2000ms);
    context.wait_for_all_requests();
    shiny_sound_handler.process_pending(context);

    // Do a fast travel outside the gate to reset spawns
    std::string extra_eror_msg = " This is after leaving zone.";
    
    // since we already set up max zoom before, we don't need to do that again when calling fast_travel_outside_zone()
    bool _go_to_max_zoom_level = false;
    fast_travel_outside_zone(env, context, wild_zone, _go_to_max_zoom_level, std::move(extra_eror_msg));
}

// After fast travel back to a wild zone, go through entrance and move forward.
// Then call `leave_zone_and_reset_spawns()` to leave zone and reset the spawns
//
// The part of moving into zone is robust against day/night change while
// `leave_zone_and_reset_spawns()` will handle the day/night change there.
//
// movement_mode:
// 0: No movement between fast travel. Always outside gate. Safe from wild pokemon
// 1: Approach the gate but don't enter. Always outside gate. Safe from wild pokemon
// 2: Enter gate and may go deep based on `walk_time_in_zone` and `running`. Not safe.
void do_one_wild_zone_trip(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    size_t movement_mode,
    Milliseconds walk_time_in_zone,
    bool running,
    WildZone wild_zone,
    ShinySoundHandler& shiny_sound_handler,
    bool to_max_zoom_level_on_map
){
    ShinyHunt_WildZoneEntrance_Descriptor::Stats& stats = env.current_stats<ShinyHunt_WildZoneEntrance_Descriptor::Stats>();
    context.wait_for_all_requests();
    shiny_sound_handler.process_pending(context);

    if (movement_mode >= 1){
        go_to_entrance(env, context);
        context.wait_for_all_requests();
        shiny_sound_handler.process_pending(context);
    }
    if (movement_mode == 2){
        // Mash button A to enter the zone.
        pbf_mash_button(context, BUTTON_A, 2000ms);
        context.wait_for_all_requests();

        {
            OverworldPartySelectionWatcher overworld;
            int ret = wait_until(
                env.console, context,
                std::chrono::milliseconds(10000ms),
                {overworld}
            );
            if (ret < 0){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Unable to detect overworld after entering zone.",
                    env.console
                );
            }
            env.console.log("Detected overworld after entering zone.");
        }
        context.wait_for(100ms);

        shiny_sound_handler.process_pending(context);
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
        shiny_sound_handler.process_pending(context);
    }

    if (movement_mode <= 1){
        // we are not in the zone. so no wild pokemon handling!
        fast_travel_outside_zone(env, context, wild_zone, to_max_zoom_level_on_map);
    }else{
        leave_zone_and_reset_spawns(
            env, context,
            walk_time_in_zone, wild_zone,
            shiny_sound_handler,
            to_max_zoom_level_on_map
        );
    }
    // wait 0.5 sec for the game to be ready to control player character again
    pbf_wait(context, 500ms);
    // Now if everything works fine, we are back at the entrance via a fast travel

    stats.visits++;
    env.update_stats();
}


void ShinyHunt_WildZoneEntrance::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);
    to_max_zoom_level_on_map = true;

    // Mash button B to let Switch register the controller
    pbf_mash_button(context, BUTTON_B, 500ms);

    ShinyHunt_WildZoneEntrance_Descriptor::Stats& stats = env.current_stats<ShinyHunt_WildZoneEntrance_Descriptor::Stats>();

    ShinySoundHandler shiny_sound_handler(SHINY_DETECTED);

    PokemonLA::ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool {
        //  Warning: This callback will be run from a different thread than this function.
        stats.shinies++;
        env.update_stats();
        env.console.overlay().add_log("Shiny Sound Detected!", COLOR_YELLOW);
       
        return shiny_sound_handler.on_shiny_sound(
            env, env.console,
            stats.shinies,
            error_coefficient
        );
    });

    int consecutive_failures = 0;
    run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            while (true){
                try{
                    do_one_wild_zone_trip(
                        env, context, 
                        MOVEMENT.current_value(), WALK_TIME_IN_ZONE, RUNNING, WILD_ZONE,
                        shiny_sound_handler,
                        to_max_zoom_level_on_map
                    );
                    // Fast travel auto saves the game. So now the map is fixed at max zoom level.
                    // We no longer needs to zoom in future.
                    to_max_zoom_level_on_map = false;
                    
                    // No failure. Reset consecutive failure counter.
                    consecutive_failures = 0;
                }catch (OperationFailedException&){
                    consecutive_failures++;
                    env.log("Consecutive failures: " + std::to_string(consecutive_failures), COLOR_RED);
                    if (consecutive_failures >= 3){
                        go_home(env.console, context);
                        throw;
                    }
                    env.log("Error encountered. Resetting...", COLOR_RED);
                    stats.game_resets++;
                    stats.errors++;
                    env.update_stats();
                    env.console.overlay().add_log("Error Found. Reset Game", COLOR_RED);
                    go_home(env.console, context);
                    reset_game_from_home(env, env.console, context);
                }
                send_program_status_notification(env, NOTIFICATION_STATUS);
            }
        },
        {{shiny_detector}}
    );
    
    //  Shiny sound detected and user requested stopping the program when
    //  detected shiny sound.
    shiny_sound_handler.process_pending(context);

    go_home(env.console, context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
