/*  Shiny Hunt - Wild Zone Entrance
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Time.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_AlertEyeDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_OverworldPartySelectionDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA/Programs/PokemonLZA_GameEntry.h"
#include "PokemonLZA_WildZoneEntrance.h"

// #include <iostream>
// using std::cout, std::endl;

namespace PokemonAutomation::NintendoSwitch::PokemonLZA {

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
                {WildZone::WILD_ZONE_20_NO_DISTORTION, "wild-zone-20", "Wild Zone 20 Without Distortion"},
                {WildZone::WILD_ZONE_20_WITH_DISTORTION, "wild-zone-20-distortion", "Wild Zone 20 With Distortion"},
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
        AllowCommandsWhenRunning::DISABLE_COMMANDS
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
        m_display_order.emplace_back("Chased");
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
            {2, "enter-zone", "Enter Zone (Don't Go Too Far or May Get Stuck by Obstacles When Running Back)"},
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
    , NOTIFICATION_STATUS("Status Update", true, false, Seconds(3600))
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
// This function is robust against day/night changes.
void go_to_entrance(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    WildZone wildzone
){
    double starting_direction = 0;
    uint8_t joystick_x = 128;
    if (wildzone == WildZone::WILD_ZONE_1){
        // The fast travel point of Wild Zone 1 does not face the zone entrance directly. We need
        // to turn a little bit to the right
        starting_direction = get_facing_direction(env.console, context);
        joystick_x = 145;
    }
    int ret = run_towards_gate_with_A_button(env.console, context, joystick_x, 0, 10s);
    switch(ret){
    case 0: // detected button A. Reached gate
        break;
    case 1: // day/night change happened.
        if (wildzone == WildZone::WILD_ZONE_1 && 
            get_angle_between_facing_directions(starting_direction, get_facing_direction(env.console, context)) > 2.5){
            joystick_x = 128; // we've already turned. Just need to go forward to enter the zone
        }
        ret = run_towards_gate_with_A_button(env.console, context, joystick_x, 0, 10s);
        if (ret != 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "go_to_entrance(): Cannot reach gate from outside after day/night change.",
                env.console
            );
        }
        break;
    default:
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "go_to_entrance(): Cannot reach gate from outside.",
            env.console
        );
    }
}


void fast_travel_outside_zone(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    WildZone wild_zone,
    bool to_max_zoom_level_on_map,
    std::string extra_error_msg = "",
    bool map_already_opened = false
){
    if (!map_already_opened){
        if (!open_map(env.console, context, to_max_zoom_level_on_map)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "fast_travel_outside_zone(): Fast travel disabled from supposedly outside the entrance." + extra_error_msg,
                env.console
            );
        }
    }

    move_map_cursor_from_entrance_to_zone(env.console, context, wild_zone);

    FastTravelState travel_status = fly_from_map(env.console, context);
    if (travel_status != FastTravelState::SUCCESS){
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

    AlertEyeDetector alert_eye_detector(COLOR_WHITE, &env.console.overlay());
    auto latest_frame = env.console.video().snapshot_latest_blocking();
    // if there is the alert symbol of a white eye telling player they are being chased by wild pokmeon
    const bool has_alert_eye = latest_frame ? alert_eye_detector.detect(latest_frame): false;

    if (!has_alert_eye){
        // we are not being chased by wild pokemon. Try to fast travel back to entrance.

        // Open map is robust against day/night change. So after open_map()
        // we are sure we are in map view
        FastTravelState travel_status = open_map_and_fly_in_place(env.console, context, to_max_zoom_level_on_map);
        // since we have set up max zoom now, we don't need to do that again when calling fast_travel_outside_zone() later
        to_max_zoom_level_on_map = false;
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
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "leave_zone_and_reset_spawns(): Cannot fast travel to zone from outside the entrance.",
                    env.console
                );
            }
            env.log("Fast travel after failed to enter zone by day/night change");
            return;
        }

        // travel_status == FastTravelState::PURSUED, we are being attacked by wild pokemon.
        // mash B to close map and return to overworld
        map_to_overworld(env.console, context);
        // if (PreloadSettings::instance().DEVELOPER_MODE){
        //     dump_debug_image(env.console, "AlertEyeDetector", "FalseNegative", latest_frame);
        // }
    }
   
    stats.chased++;
    env.update_stats();

    walk_time_in_zone += 2s; // give some extra time
    env.log("Escaping");
    env.console.overlay().add_log("Escaping Back to Entrance");
    
    const double starting_direction = get_facing_direction(env.console, context);

    int ret = run_towards_gate_with_A_button(env.console, context, 128, 255, walk_time_in_zone);
    switch (ret){
    case 0: // Found button A. Reached the gate.
        break;
    case 1: // Day/night change happened.
        {
            const double cur_direction = get_facing_direction(env.console, context);
            const double direction_change = get_angle_between_facing_directions(starting_direction, cur_direction);
            env.log("Facing direction difference after day/night change: " + tostr_fixed(direction_change, 0) + " deg, from "
                + tostr_fixed(starting_direction, 0) + " to " + tostr_fixed(cur_direction, 0) + " deg");
            
            uint8_t joystick_y = 0;
            if (direction_change > 150.0){
                // we are facing towards the gate
                env.log("Running forward");
                env.console.overlay().add_log("Running Forward");
                joystick_y = 0;
            }else if(direction_change < 30.0){
                // we are facing away from the gate
                env.log("Running back");
                env.console.overlay().add_log("Running Back");
                joystick_y = 255;
            }else{
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "leave_zone_and_reset_spawns(): Facing direction after day/night change is wrong: " + tostr_fixed(direction_change, 0) + " deg",
                    env.console
                );
            }

            // Running forward or backward depends on character facing to go back to zone entrance
            ret = run_towards_gate_with_A_button(env.console, context, 128, joystick_y, walk_time_in_zone);
            if (ret != 0){
                stats.errors++;
                env.update_stats();
                throw UserSetupError(
                    env.logger(),
                    "Program stuck in the zone while escaping from wild pokemon. "
                    "Pick a path that won't get you stuck by terrain or obstacles."
                );
            }
        }
        break;
    default:
        stats.errors++;
        env.update_stats();
        throw UserSetupError(
            env.logger(),
            "Program stuck in the zone while escaping from wild pokemon. "
            "Pick a path that won't get you stuck by terrain or obstacles."
        );
    }
    shiny_sound_handler.process_pending(context);

    // Found button A, so we are at the entrance.
    // Mash A to leave Zone.

    std::string extra_eror_msg = " This is after leaving zone.";

    // Due to day/night change may eating the mashing button A sequence, we may still be inside the zone!
    // We need to check if we can fast travel 
    if (leave_zone_gate(env.console, context)){
        shiny_sound_handler.process_pending(context);
        // Do a fast travel outside the gate to reset spawns
        fast_travel_outside_zone(env, context, wild_zone, to_max_zoom_level_on_map, std::move(extra_eror_msg));
        return;
    }

    // there is a day/night change while leaving the zone. We don't know if we are still inside the zone.
    //
    // We can't call leave_zone_gate() again because at Wild Zone 4 there are npcs outside the gate that can
    // press A to talk to. If the program can get stuck into talking to the npc.

    FastTravelState travel_status = open_map_and_fly_in_place(env.console, context, to_max_zoom_level_on_map);
    if (travel_status == FastTravelState::SUCCESS){
        // We can fast travel and we fast traveled. This means we were inside the gate but now safe.
        env.log("We fast traveled. We were inside the gate but now safe for next trip");
        return;
    }else if(travel_status == FastTravelState::NOT_AT_FLY_SPOT){
        env.log("We cannot fast travel in place. We left zone successfully");
        // we cannot fast travel at current location. So we have left the zone!
        // Fast travel to the zone gate to reset spawn
        const bool map_already_opened = true;
        fast_travel_outside_zone(env, context, wild_zone, to_max_zoom_level_on_map,
            std::move(extra_eror_msg), map_already_opened);
        return;
    }
    // We cannot fast travel: we are still being chased by wild pokemon
    env.log("We cannot fast travel. Still chased by pokemon");
    
    // Mash B to close map and return to overworld
    map_to_overworld(env.console, context);
    // Mash A to leave zone gate
    env.log("Mashing A again to leave zone");
    leave_zone_gate(env.console, context);
    shiny_sound_handler.process_pending(context);
    // Do a fast travel outside the gate to reset spawns
    env.log("Finally, we should have left the zone");
    fast_travel_outside_zone(env, context, wild_zone, to_max_zoom_level_on_map, std::move(extra_eror_msg));
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
    env.log("Starting one wild zone trip");
    ShinyHunt_WildZoneEntrance_Descriptor::Stats& stats = env.current_stats<ShinyHunt_WildZoneEntrance_Descriptor::Stats>();
    context.wait_for_all_requests();
    shiny_sound_handler.process_pending(context);

    if (movement_mode >= 1){
        go_to_entrance(env, context, wild_zone);
        context.wait_for_all_requests();
        shiny_sound_handler.process_pending(context);
    }
    if (movement_mode == 2){
        // Mash button A to enter the zone.
        pbf_mash_button(context, BUTTON_A, 2000ms);
        context.wait_for_all_requests();

        // Wait for the overworld party view to be back. That is when
        // the player is given control again after the entering gate animation.
        // We use 50s here to account for day night change
        wait_until_overworld(env.console, context, 50s);
        env.console.log("Detected overworld after entering zone.");

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
            pbf_move_left_joystick(context, {0, +1}, walk_time_in_zone, 200ms);
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

    // Now if everything works fine, we are back at the entrance via a fast travel

    stats.visits++;
    env.update_stats();
}


void ShinyHunt_WildZoneEntrance::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    //  Record when we should zoom out the map for flyable fast travel icon
    //  detection on map during fast traveling.
    bool to_max_zoom_level_on_map = true;

    //  Mash button B to let Switch register the controller
    pbf_mash_button(context, BUTTON_B, 500ms);

    //  Detect direction arrow to test the detector
    //  m_starting_angle = get_current_facing_angle(env, context);

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
                        if (PreloadSettings::instance().DEVELOPER_MODE && GlobalSettings::instance().SAVE_DEBUG_VIDEOS_ON_SWITCH){
                            env.log("Saving debug video on Switch...");
                            env.console.overlay().add_log("Save Debug Video on Switch");
                            pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 0ms);
                            context.wait_for_all_requests();
                        }
                        go_home(env.console, context); // go Home to preserve game state for debugging
                        throw;
                    }
                    env.log("Error encountered. Resetting...", COLOR_RED);
                    stats.game_resets++;
                    stats.errors++;
                    env.update_stats();
                    env.console.overlay().add_log("Error Found. Reset Game", COLOR_RED);
                    go_home(env.console, context);
                    reset_game_from_home(env, env.console, context);
                    to_max_zoom_level_on_map = true; // map zoom level reset after game reset
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


}  // namespace PokemonAutomation::NintendoSwitch::PokemonLZA
