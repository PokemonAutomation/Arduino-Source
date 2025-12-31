/*  Shiny Hunt - Wild Zone Cafe
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
#include "PokemonLZA_WildZoneCafe.h"

namespace PokemonAutomation::NintendoSwitch::PokemonLZA {

using namespace Pokemon;


ShinyHunt_WildZoneCafe_Descriptor::ShinyHunt_WildZoneCafe_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:ShinyHunt-WildZoneCafe", STRING_POKEMON + " LZA",
        "Wild Zone Cafe",
        "Programs/PokemonLZA/ShinyHunt-WildZoneCafe.html",
        "Shiny hunt by repeatedly entering Wild Zone cafe.",
        ProgramControllerClass::StandardController_NoRestrictions, FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

class ShinyHunt_WildZoneCafe_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : visits(m_stats["Visits"])
        , chased(m_stats["Chased"])
        , shinies(m_stats["Shiny Sounds"])
        , game_resets(m_stats["Game Resets"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Visits");
        m_display_order.emplace_back("Chased");
        m_display_order.emplace_back("Shiny Sounds");
        m_display_order.emplace_back("Game Resets");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& visits;
    std::atomic<uint64_t>& chased;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& game_resets;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> ShinyHunt_WildZoneCafe_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


ShinyHunt_WildZoneCafe::ShinyHunt_WildZoneCafe()
    : CAFE(
        "<b>Caf\u00e9:</b>",
        {
            {WildZoneCafe::CAFE_BATAILLE, "cafe-bataille", "Wild Zone 6 - Caf\u00e9 Bataille"},
            {WildZoneCafe::CAFE_ULTIMO, "cafe-ultimo", "Wild Zone 15 - Caf\u00e9 Ultimo"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        WildZoneCafe::CAFE_BATAILLE
    )
    , NUM_VISITS("<b>Number of Visits:</b><br>Stop after this many visits. 0 means no limit.", LockMode::UNLOCK_WHILE_RUNNING, 0)
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
    PA_ADD_OPTION(CAFE);
    PA_ADD_OPTION(NUM_VISITS);
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void do_one_cafe_trip(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    WildZoneCafe cafe,
    ShinySoundHandler& shiny_sound_handler,
    bool to_zoom_to_max
){
    env.log("Starting one cafe trip");
    ShinyHunt_WildZoneCafe_Descriptor::Stats& stats = env.current_stats<ShinyHunt_WildZoneCafe_Descriptor::Stats>();
    context.wait_for_all_requests();
    shiny_sound_handler.process_pending(context);

    AlertEyeDetector alert_eye_detector(COLOR_WHITE, &env.console.overlay());
    auto latest_frame = env.console.video().snapshot_latest_blocking();
    // if there is the alert symbol of a white eye telling player they are being chased by wild pokmeon
    const bool has_alert_eye = latest_frame ? alert_eye_detector.detect(latest_frame): false;

    if (!has_alert_eye){
        // we are not being chased by wild pokemon. Try to fast travel back to entrance.

        // Open map is robust against day/night change. So after open_map()
        // we are sure we are in map view
        bool can_fast_travel = open_map(env.console, context, to_zoom_to_max);
        to_zoom_to_max = false;

        if (can_fast_travel){
            pbf_wait(context, 300ms);
            switch(cafe){
            case WildZoneCafe::CAFE_BATAILLE:
                env.log("Move to Cafe Bataille icon");
                pbf_move_left_joystick(context, {+0.157, +0.844}, 100ms, 0ms);
                break;
            case WildZoneCafe::CAFE_ULTIMO:
                env.log("Move to Cafe Ultimo icon");
                pbf_move_left_joystick(context, {-0.609, +0.219}, 100ms, 0ms);
                break;
            }
            pbf_wait(context, 300ms);
            FastTravelState travel_status = fly_from_map(env.console, context);
            if (travel_status == FastTravelState::SUCCESS){
                env.log("Fast travel back to cafe");
                return;
            } else if (travel_status == FastTravelState::NOT_AT_FLY_SPOT){
                stats.errors++;
                env.update_stats();
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "do_one_cafe_trip: Cannot fast travel after moving map cursor.",
                    env.console
                );
            }
            // else: travel_status == FastTravelState::PURSUED
        }

        // if (PreloadSettings::instance().DEVELOPER_MODE){
        //     dump_debug_image(env.console, "AlertEyeDetector", "FalseNegative", latest_frame);
        // }
    }
    // we are being chased by wild pokemon

    stats.chased++;
    env.update_stats();
    env.log("Escaping");
    env.console.overlay().add_log("Escaping to Entrance");
    const double starting_direction = get_facing_direction(env.console, context);

    double move_x = -1, move_y = +1;
    switch(cafe){
    case WildZoneCafe::CAFE_BATAILLE:
        env.log("Move to zone gate from Cafe Bataille");
        move_x = +1; move_y = -0.25;
        break;
    case WildZoneCafe::CAFE_ULTIMO:
        env.log("Move to zone gate from Cafe Ultimo");
        move_x = 0; move_y = -1;
        break;
    }

    int ret = run_towards_gate_with_A_button(env.console, context, move_x, move_y, Seconds(10));
    switch (ret){
    case 0: // Found button A. Reached the gate.
        break;
    case 1: // day night change happend
        {
            const double cur_direction = get_facing_direction(env.console, context);
            const double direction_change = get_angle_between_facing_directions(starting_direction, cur_direction);
            env.log("Facing direction difference after day/night change: " + tostr_fixed(direction_change, 0) + " deg, from "
                + tostr_fixed(starting_direction, 0) + " to " + tostr_fixed(cur_direction, 0) + " deg");

            if (direction_change > 30){
                // we are already facing the gate
                move_x = 0; move_y = +1;
                env.log("Running forward");
                env.console.overlay().add_log("Running Forward");
            }
            // Running forward or backward depends on character facing to go back to zone entrance
            ret = run_towards_gate_with_A_button(env.console, context, move_x, move_y, Seconds(10));
            if (ret != 0){
                stats.errors++;
                env.update_stats();
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "do_one_cafe_trip: Cannot reach wild zone gate after day/night change.",
                    env.console
                );
            }
        }
        break;
    default:
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "do_one_cafe_trip: Cannot reach wild zone gate after being chased by wild pokemon.",
            env.console
        );
    }
    shiny_sound_handler.process_pending(context);

    // Found button A, so we are at the entrance.
    // Mash A to leave Zone.
    if (!leave_zone_gate(env.console, context)){
        // day/night change happens while leaving
        // Try leave again
        leave_zone_gate(env.console, context);
    }
    shiny_sound_handler.process_pending(context);

    // do a fast travel back to cafe
    bool can_fast_travel = open_map(env.console, context, to_zoom_to_max);
    if (!can_fast_travel){
        // cannot fast travel outside zone. Chased by wild pokemon that used Dig?
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "do_one_cafe_trip: Cannot fast travel outside gate.",
            env.console
        );
    }
    
    // move map cursor to cafe:
    pbf_wait(context, 300ms);
    switch(cafe){
    case WildZoneCafe::CAFE_BATAILLE:
        env.log("Move to Cafe Bataille icon from zone gate");
        pbf_move_left_joystick(context, {-0.25, -0.252}, 100ms, 0ms);
        break;
    case WildZoneCafe::CAFE_ULTIMO:
        env.log("Move to Cafe Ultimo icon from zone gate");
        pbf_move_left_joystick(context, {+0.488, -0.126}, 100ms, 0ms);
        break;
    }
    pbf_wait(context, 300ms);

    FastTravelState travel_status = fly_from_map(env.console, context);
    if (travel_status != FastTravelState::SUCCESS){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "do_one_cafe_trip: Cannot fast travel to cafe.",
            env.console
        );
    }
}


void ShinyHunt_WildZoneCafe::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    //  Record when we should zoom out the map for flyable fast travel icon
    //  detection on map during fast traveling.
    bool to_max_zoom_level_on_map = true;

    // Mash button B to let Switch register the controller
    pbf_mash_button(context, BUTTON_B, 500ms);

    ShinyHunt_WildZoneCafe_Descriptor::Stats& stats = env.current_stats<ShinyHunt_WildZoneCafe_Descriptor::Stats>();

    ShinySoundHandler shiny_sound_handler(SHINY_DETECTED);

    PokemonLA::ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool {
        // Warning: This callback will be run from a different thread than this function.
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
                    do_one_cafe_trip(
                        env, context,
                        CAFE, shiny_sound_handler, to_max_zoom_level_on_map
                    );
                    // Fast travel auto saves the game. So now the map is fixed at max zoom level.
                    // We no longer needs to zoom in future.
                    to_max_zoom_level_on_map = false;
                    stats.visits++;
                    env.update_stats();
                    if (NUM_VISITS > 0 && stats.visits >= NUM_VISITS){
                        break;
                    }
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
                }
                send_program_status_notification(env, NOTIFICATION_STATUS);
            }
        },
        {{shiny_detector}}
    );

    // Shiny sound detected and user requested stopping the program when
    // detected shiny sound.
    shiny_sound_handler.process_pending(context);

    go_home(env.console, context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}  // namespace PokemonAutomation::NintendoSwitch::PokemonLZA
