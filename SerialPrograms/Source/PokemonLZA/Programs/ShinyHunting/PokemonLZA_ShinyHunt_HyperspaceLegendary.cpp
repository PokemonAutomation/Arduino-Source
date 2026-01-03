/*  Shiny Hunt - Hyperspace Legendary
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "ML/Inference/ML_YOLOv5Detector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_HyperspaceCalorieDetector.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA/Programs/PokemonLZA_GameEntry.h"
#include "PokemonLZA_ShinyHunt_HyperspaceLegendary.h"

#include <format>

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {

using namespace Pokemon;
using ML::YOLOv5Watcher;
using DetectionBox = ML::YOLOv5Session::DetectionBox;


ShinyHunt_HyperspaceLegendary_Descriptor::ShinyHunt_HyperspaceLegendary_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:ShinyHunt-HyperspaceLegendary", STRING_POKEMON + " LZA",
        "Hyperspace Legendary",
        "Programs/PokemonLZA/ShinyHunt-HyperspaceLegendary.html",
        "Shiny hunt legendary " + STRING_POKEMON + " in Hyperspace.",
        ProgramControllerClass::StandardController_NoRestrictions, FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS, {}
    )
{}

class ShinyHunt_HyperspaceLegendary_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : spawns(m_stats["Spawns"])
        , game_resets(m_stats["Game Resets"])
        , shinies(m_stats["Shiny Sounds"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Spawns");
        m_display_order.emplace_back("Game Resets");
        m_display_order.emplace_back("Shiny Sounds");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& spawns;
    std::atomic<uint64_t>& game_resets;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> ShinyHunt_HyperspaceLegendary_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


ShinyHunt_HyperspaceLegendary::ShinyHunt_HyperspaceLegendary()
    : SHINY_DETECTED("Shiny Detected", "", "2000 ms", ShinySoundDetectedAction::STOP_PROGRAM)
    , LEGENDARY("<b>Legendary " + STRING_POKEMON + ":</b>",
        {
            {Legendary::COBALION, "cobalion", "Cobalion"},
            {Legendary::VIRIZION,  "virizion",  "Virizion"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Legendary::VIRIZION
    )
    , MIN_CALORIE_TO_CATCH(
        "<b>Minimum Cal. Reserved to Catch Legendary:</b><br>If applicable, the program will stop refreshing the Legendary spawn to give this amount of Calorie left for catching the Legendary."
        "<br>NOTE: use 5-star donut for best catch chance and enough time in the Legendary hyperspace."
        "<br>Cal. per sec: 1 Star: 1 Cal./s, 2 Star: 1.6 Cal./s, 3 Star: 3.5 Cal./s, 4 Star: 7.5 Cal./s, 5 Star: 10 Cal./s",
        LockMode::UNLOCK_WHILE_RUNNING,
        600, 0, 9999 // default, min, max
    )
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
    PA_ADD_OPTION(LEGENDARY);
    PA_ADD_OPTION(MIN_CALORIE_TO_CATCH);
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}

namespace {

// Return true if the Calorie number on screen <= min_calorie
bool check_calorie(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    HyperspaceCalorieWatcher& calorie_watcher,
    uint16_t min_calorie,
    uint16_t additional_calorie_buffer
){
    int ret = wait_until(
        env.console, context, std::chrono::seconds(1), {calorie_watcher}
    );
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "hunt_virizion_balcony(): does not detect Calorie number after waiting for a second",
            env.console
        );
    }

    const uint16_t calorie_number = calorie_watcher.calorie_number();
    const std::string log_msg = std::format("Calorie: {}/{}", calorie_number, min_calorie);
    env.add_overlay_log(log_msg);
    env.log(log_msg);
    if (calorie_number <= min_calorie + additional_calorie_buffer){
        if (additional_calorie_buffer == 0){
            env.log("min calorie reached");
            env.add_overlay_log("Min Calorie Reached");
        } else{
            env.log("Close to min Calorie");
            env.add_overlay_log("Close to Min Calorie");
        }
        return true;
    }
    return false;
}


// Wait until the warp pad is detected
void detect_warp_pad(SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    ShinyHunt_HyperspaceLegendary_Descriptor::Stats& stats){

    ButtonWatcher ButtonA(
        COLOR_RED,
        ButtonType::ButtonA,
        {0.4, 0.1, 0.2, 0.8},
        &env.console.overlay()
    );

    int ret = wait_until(
        env.console, context, 10s,
        {ButtonA}
    );
    if (ret < 0){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "route_terrakion_reset(): Cannot detect warp pad after 10 seconds",
            env.console
        );
    }else{
        env.log("Detected warp pad.");
        // env.console.overlay().add_log("Warp Pad Detected");
    }
}

void hunt_cobalion(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    ShinyHunt_HyperspaceLegendary_Descriptor::Stats& stats,
    SimpleIntegerOption<uint16_t>& MIN_CALORIE_TO_CATCH)
{

    // Spawn refreshing loop takes 14 sec. Going to check Cobalion takes 13 sec.
    // 10 for 10 cal per sec
    const uint16_t min_calorie = MIN_CALORIE_TO_CATCH + (14 + 13) * 10;

    HyperspaceCalorieWatcher calorie_watcher(env.logger());
    while(true){
        // run to the right to spawn in Cobalion
        ssf_press_button(context, BUTTON_B, 0ms, 6000ms, 0ms);
        pbf_move_left_joystick(context, {+1, 0}, 7000ms, 0ms); 
        pbf_wait(context, 100ms);
        // run to the left to despawn
        // add 20ms to try to ensure drift never prevents despawning
        ssf_press_button(context, BUTTON_B, 0ms, 6000ms, 0ms);
        pbf_move_left_joystick(context, {-1, 0}, 7020ms, 0ms);
        pbf_wait(context, 100ms);

        context.wait_for_all_requests();

        stats.spawns++;
        env.update_stats();
        if (check_calorie(env, context, calorie_watcher, min_calorie)){
            break;
        }
    }

    // Run to Cobalion to trigger potential shiny sound
    env.log("Move to check Cobalion.");
    env.add_overlay_log("To Check Cobalion");

    // run right to line up with Cobalion
    ssf_press_button(context, BUTTON_B, 0ms, 10000ms, 0ms);
    pbf_move_left_joystick(context, {+1, 0}, 8370ms, 0ms); 
    // run forward to trigger potential shiny sound
    pbf_move_left_joystick(context, {0, +1}, 4500ms, 0ms); 

    context.wait_for_all_requests();
}

// Use teleport pad to refresh Terrakion spawns until MIN_CALORIE is reached.
// Then move close to Terrakion so the shiny sound detector (from the caller level)
// can detect shiny and stop program.
// This function always returns false to mean it will not require the program to
// stop, as it relies on the shiny sound detector from the caller level.
void hunt_terrakion(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    ShinyHunt_HyperspaceLegendary_Descriptor::Stats& stats,
    SimpleIntegerOption<uint16_t>& MIN_CALORIE_TO_CATCH)
{
    HyperspaceCalorieWatcher calorie_watcher(env.logger());
    while(true){
        // Warp away from Terrakion to despawn
        detect_warp_pad(env, context, stats);
        pbf_press_button(context, BUTTON_A, 160ms, 80ms);

        // Warp towards Terrakion
        detect_warp_pad(env, context, stats);
        pbf_press_button(context, BUTTON_A, 160ms, 80ms);

        // Roll and roll back on Terrakion's roof to respawn
        detect_warp_pad(env, context, stats);
        pbf_press_button(context, BUTTON_Y, 100ms, 900ms);
        pbf_move_left_joystick(context, {0, -1}, 80ms, 160ms);
        pbf_press_button(context, BUTTON_Y, 100ms, 900ms);
        
        context.wait_for_all_requests();

        stats.spawns++;
        env.update_stats();

        // Spawn refreshing loop takes 3 sec. Going to check Virizion takes 8 sec.
        // 10 for 10 cal per sec
        if (check_calorie(env, context, calorie_watcher, MIN_CALORIE_TO_CATCH, (3 + 8) * 10)){
            break;
        }
    }

    // Use warp pads to reset position
    detect_warp_pad(env, context, stats);
    pbf_press_button(context, BUTTON_A, 160ms, 80ms);
    detect_warp_pad(env, context, stats);
    pbf_press_button(context, BUTTON_A, 160ms, 80ms);
    detect_warp_pad(env, context, stats);

    // Roll to Terrakion to trigger potential shiny sound
    env.log("Move to check Terrakion.");
    env.add_overlay_log("To Check Terrakion");

    pbf_press_button(context, BUTTON_Y, 100ms, 900ms);
    pbf_press_button(context, BUTTON_Y, 100ms, 900ms);
    pbf_move_left_joystick(context, {-1, 1}, 80ms, 160ms);
    pbf_press_button(context, BUTTON_Y, 100ms, 900ms);
    pbf_move_left_joystick(context, {0, 1}, 80ms, 500ms);
    pbf_press_button(context, BUTTON_Y, 100ms, 900ms);
    pbf_move_left_joystick(context, {1, 1}, 80ms, 160ms);
    pbf_press_button(context, BUTTON_Y, 100ms, 900ms);
    pbf_press_button(context, BUTTON_Y, 100ms, 900ms);
    pbf_press_button(context, BUTTON_Y, 100ms, 900ms);

    context.wait_for_all_requests();
}

#if 0
// TODO: WIP: use OpenCV to traverse the alley to move to check
// Virizion.
void hunt_virizion_balcony(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    ShinyHunt_HyperspaceLegendary_Descriptor::Stats& stats,
    SimpleIntegerOption<uint16_t>& MIN_CALORIE_TO_CATCH)
{

    std::string model_path = "PokemonLZA/YOLO/Virizion.onnx";
    YOLOv5Watcher yolo_watcher(env.console.overlay(), model_path);

    const uint16_t min_calorie = MIN_CALORIE_TO_CATCH;

    // running forward
    const Milliseconds run_duration(4400);

    HyperspaceCalorieWatcher calorie_watcher(env.logger());
    while(true){
        // running forward
        ssf_press_button(context, BUTTON_B, 0ms, 2*run_duration, 0ms);
        // Add 30 ms to avoid any drift using the balustrade
        pbf_move_left_joystick(context, {0, +1}, run_duration + 30ms, 0ms);
        // run back
        pbf_move_left_joystick(context, {0, -1}, run_duration, 0ms);
        // Wait for a short time to allow the game to register the next button B press
        pbf_wait(context, 100ms);
        context.wait_for_all_requests();

        stats.spawns++;
        env.update_stats();
    
        if (check_calorie(env, context, calorie_watcher, min_calorie)){
            break;
        }
    }

    env.log("Move to check Virizion");
    env.add_overlay_log("To Check Virision");
    // We have done enough shuttle runs to refresh Virizion spawns.
    // Now run towards it to check shiny!
    
    // Push left joystick rightward to let the character face right
    pbf_move_left_joystick(context, {+1, 0}, 100ms, 0ms);
    // Roll once to leave the balcony area
    pbf_press_button(context, BUTTON_Y, 100ms, 1s);
    context.wait_for_all_requests();

    size_t trash_bin_idx = yolo_watcher.label_index("trash-bin");

    run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            // Run downstairs towards the trash bin
            ssf_press_button(context, BUTTON_B, 0ms, Seconds(1), 0ms);
            pbf_move_left_joystick(context, {0, +1}, Seconds(5), 0ms);
            context.wait_for_all_requests();

            while(true){
                const std::vector<DetectionBox>& detections = yolo_watcher.detected_boxes();
                const DetectionBox* detection = find_detection(detections, trash_bin_idx);
                if (detection == nullptr){
                    context.wait_for(100ms);
                    continue;
                }

                double center_x = detection->box.x + detection->box.width/2;

                env.log("Found trash bin");
                env.add_overlay_log(std::format("Found Trash Bin at {:.2f}", center_x));
                if (0.45 <= center_x && center_x <= 0.55){
                    // We are facing the trash bin, stop
                    env.add_overlay_log("Facing Trash Bin");
                    break;
                }
                double dir_x = (center_x < 0.5 ? -0.5 : 0.5);
                int duration = static_cast<int>(std::fabs(center_x - 0.5) * 1000);
                pbf_move_right_joystick(context, {dir_x, 0.0}, Milliseconds(duration), 0ms);
                context.wait_for_all_requests();
            }
        },
        {{yolo_watcher}}
    );
}
#endif


// Use shuttle run on rooftop to refresh Virizion spawns until MIN_CALORIE is reached.
// Then move close to Virizion so the shiny sound detector (from the caller level)
// can detect shiny and stop program
void hunt_virizion_rooftop(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    ShinyHunt_HyperspaceLegendary_Descriptor::Stats& stats,
    SimpleIntegerOption<uint16_t>& MIN_CALORIE_TO_CATCH)
{
    auto climb_ladder = [&](Milliseconds hold){
        pbf_move_left_joystick(context, {0.0, 1.0}, hold, 0ms);
    };
    auto run_forward = [&](Milliseconds hold){
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, {0.0, 1.0}, {0.0, 0.0}, hold);
    };
    auto run_backward = [&](Milliseconds hold){
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, {0.0, -1.0}, {0.0, 0.0}, hold);
    };
    auto change_character_facing_direction = [&](double left_joystick_x, double left_joystick_y){
        pbf_move_left_joystick(context, {left_joystick_x, left_joystick_y}, 500ms, 0ms);
    };
    auto run_changing_direction = [&](Milliseconds hold, double right_joystick_x){
        pbf_controller_state(context, BUTTON_B, DPAD_NONE, {0.0, 1.0}, {right_joystick_x, 0.0}, hold);
    };

    // Starting facing the ladder
    HyperspaceCalorieWatcher calorie_watcher(env.logger());
    // This loop takes about 15 sec
    while(true){
        pbf_press_button(context, BUTTON_A, 100ms, 500ms); // hop on ladder
        climb_ladder(2800ms);
        run_forward(2500ms);
        run_backward(3000ms);
        pbf_wait(context, 1s); // wait for drop to lower level
        run_backward(2000ms);
        run_forward(2500ms);
        context.wait_for_all_requests();
        stats.spawns++;
        env.update_stats();
        // Spawn refreshing loop takes 15 sec. Going to check Virizion takes 22 sec.
        // 10 for 10 cal per sec
        if (check_calorie(env, context, calorie_watcher, MIN_CALORIE_TO_CATCH, (15 + 22) * 10)){
            break;
        }
    }

    // This movement to Virizion takes about 22 sec
    pbf_press_button(context, BUTTON_A, 100ms, 500ms);
    climb_ladder(2800ms);
    run_forward(2500ms);
    change_character_facing_direction(0.0, -1.0); // face backwards
    // align camera to face what character is facing
    pbf_press_button(context, BUTTON_L, 200ms, 800ms);
    
    context.wait_for_all_requests();    
    env.log("Move to check Virizion");
    env.add_overlay_log("To Check Virizion");

    run_forward(2600ms);
    pbf_wait(context, 1s); // wait for drop to lower level
    run_changing_direction(3000ms, -0.15);
    pbf_controller_state(context, BUTTON_A, DPAD_NONE, {0.0, 1.0}, {0.0, 0.0}, 2500ms);
    run_forward(5s);
    context.wait_for_all_requests();
}


} // namespace

void ShinyHunt_HyperspaceLegendary::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    // Mash button B to let Switch register the controller
    pbf_mash_button(context, BUTTON_B, 200ms);

    ShinyHunt_HyperspaceLegendary_Descriptor::Stats& stats = env.current_stats<ShinyHunt_HyperspaceLegendary_Descriptor::Stats>();

    uint8_t shiny_count = 0;
    float shiny_coefficient = 1.0;
    PokemonLA::ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool {
        //  Warning: This callback will be run from a different thread than this function.
        shiny_count++;
        stats.shinies++;
        env.update_stats();
        env.console.overlay().add_log("Shiny Sound Detected!", COLOR_YELLOW);
        shiny_coefficient = error_coefficient;
        return true;
    });

    while (true){
        const int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                if (LEGENDARY == Legendary::VIRIZION){
                    hunt_virizion_rooftop(env, context, stats, MIN_CALORIE_TO_CATCH);
                } else if (LEGENDARY == Legendary::TERRAKION){
                    hunt_terrakion(env, context, stats, MIN_CALORIE_TO_CATCH);
                } else if (LEGENDARY == Legendary::COBALION){
                    hunt_cobalion(env, context, stats, MIN_CALORIE_TO_CATCH);
                } else {
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "legendary hunt not implemented",
                        env.console
                    );
                }
            },
            {{shiny_detector}}
        ); // end run_until()
        shiny_detector.throw_if_no_sound();

        if (ret == 0 && SHINY_DETECTED.on_shiny_sound(
            env, env.console, context,
            shiny_count,
            shiny_coefficient
        )){
            break;
        }

        // no shiny sound detected or no shiny legendary detected. Reset game
        go_home(env.console, context);
        reset_game_from_home(env, env.console, context);
        stats.game_resets++;
        env.update_stats();
        if (stats.game_resets % 10 == 0){
            send_program_status_notification(env, NOTIFICATION_STATUS);
        }
    } // end while (true)

    go_home(env.console, context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
