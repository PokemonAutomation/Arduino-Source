/*  Shiny Hunt - Terrakion
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_OverworldPartySelectionDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_HyperspaceCalorieDetector.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA/Programs/PokemonLZA_GameEntry.h"
#include "PokemonLZA_ShinyHunt_Terrakion.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {

using namespace Pokemon;


ShinyHunt_Terrakion_Descriptor::ShinyHunt_Terrakion_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:ShinyHunt-Terrakion",
        STRING_POKEMON + " LZA",
        "Shiny Hunt - Terrakion",
        "Programs/PokemonLZA/ShinyHunt-Terrakion.html",
        "Shiny hunt Terrakion in Hyperspace Lumiose",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}


class ShinyHunt_Terrakion_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : shinies(m_stats["Shinies"])
        , resets(m_stats["Resets"])
        , checks(m_stats["Checks"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Checks");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& checks;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> ShinyHunt_Terrakion_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


ShinyHunt_Terrakion::ShinyHunt_Terrakion()
    : THRESHOLD("<b>Calorie Threshold:</b><br>Stop resetting and check Terrakion when this many calories remain", LockMode::UNLOCK_WHILE_RUNNING, 1500, 200, 4800)
    , SAVE_ON_START("<b>Save on Start:</b><br>Save the game when starting the program. Each cycle will start at the calorie count at the time of the save", LockMode::LOCK_WHILE_RUNNING, true)
    , SHINY_DETECTED("Shiny Detected", "", "1000 ms", ShinySoundDetectedAction::STOP_PROGRAM)
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_STATIC(SHINY_REQUIRES_AUDIO);
    PA_ADD_OPTION(THRESHOLD);
    PA_ADD_OPTION(SAVE_ON_START);
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}


bool are_calories_under_threshold(SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    ShinyHunt_Terrakion_Descriptor::Stats& stats, SimpleIntegerOption<uint16_t>& THRESHOLD){
        
    OverworldPartySelectionWatcher overworld(COLOR_WHITE, &env.console.overlay());
    int ret = wait_until(
        env.console, context, 10s,
        {overworld}
    );
    if (ret < 0){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "route_terrakion_reset(): Cannot detect overworld after warp pad.",
            env.console
        );
    }
    std::shared_ptr<const ImageRGB32> overworld_screen = overworld.last_detected_frame();
    if (overworld_screen == nullptr){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "route_terrakion_reset(): Failed to get overworld screen!",
            env.console
        );
    }
    HyperspaceCalorieDetector hyperspace_calorie_detector(env.logger());
    if (!hyperspace_calorie_detector.detect(*overworld_screen)){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "route_hyperspace_wild_zone(): Cannot read Calorie number on screen.",
            env.console
        );
    }
    const uint16_t calorie_number = hyperspace_calorie_detector.calorie_number();
    const uint16_t min_calorie = THRESHOLD;
    const std::string log_msg = std::format("Calories: {}/{}", calorie_number, min_calorie);
    env.add_overlay_log(log_msg);
    env.log(log_msg);
    if (calorie_number <= min_calorie){
        env.log("Calorie threshold reached. Stopping resets to check Terrakion.", COLOR_PURPLE);
        return true;
    }
    return false;
}


void detect_warp_pad(SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    ShinyHunt_Terrakion_Descriptor::Stats& stats){

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


// void wait_until_overworld(SingleSwitchProgramEnvironment& env, ProControllerContext& context,
//     ShinyHunt_Terrakion_Descriptor::Stats& stats) {

//     OverworldPartySelectionOverWatcher overworldWatcher(
//         COLOR_GREEN,
//         &env.console.overlay()
//     );
//     int ret = wait_until(
//         env.console, context, 10s,
//         {overworldWatcher}
//     );
//     if (ret < 0){
//         stats.errors++;
//         env.update_stats();
//         OperationFailedException::fire(
//             ErrorReport::SEND_ERROR_REPORT,
//             "wait_until_overworld(): Cannot detect overworld after 10 seconds",
//             env.console
//         );
//     } else {
//         env.log("Detected overworld.");
//         env.console.overlay().add_log("Overworld Detected");
//     }
// }


void route_reset_terrakion(SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    ShinyHunt_Terrakion_Descriptor::Stats& stats, bool do_rolls=true){

    context.wait_for_all_requests();

    // Warp away from Terrakion to despawn
    detect_warp_pad(env, context, stats);
    pbf_press_button(context, BUTTON_A, 160ms, 80ms);

    // Warp towards Terrakion
    detect_warp_pad(env, context, stats);
    pbf_press_button(context, BUTTON_A, 160ms, 80ms);

    // Roll and roll back on Terrakion's roof to respawn
    detect_warp_pad(env, context, stats);
    if (do_rolls){
        pbf_press_button(context, BUTTON_Y, 100ms, 900ms);
        pbf_move_left_joystick(context, {0, -1}, 80ms, 160ms);
        pbf_press_button(context, BUTTON_Y, 100ms, 900ms);
    }
    context.wait_for_all_requests();
    env.add_overlay_log("Respawned Terrakion");
    env.log("Respawned Terrakion.");
    stats.resets++;
    env.update_stats();
}


void route_check_terrakion(SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    ShinyHunt_Terrakion_Descriptor::Stats& stats){

    context.wait_for_all_requests();

    // Roll to Terrakion to trigger potential shiny sound
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

    env.add_overlay_log("Checking for Shiny");
    env.log("Checking shiny status of Terrakion.");
    context.wait_for_all_requests();
    stats.checks++;
    env.update_stats();
}

void ShinyHunt_Terrakion::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ShinyHunt_Terrakion_Descriptor::Stats& stats = env.current_stats<ShinyHunt_Terrakion_Descriptor::Stats>();
    ShinySoundHandler shiny_sound_handler(SHINY_DETECTED);
    PokemonLA::ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool{
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

    if (SAVE_ON_START){
        save_game_to_menu(env.console, context);
        pbf_mash_button(context, BUTTON_B, 2000ms);
    }

    int consecutive_failures = 0;
    run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            while (true){
                context.wait_for_all_requests();
                shiny_sound_handler.process_pending(context);

                try{
                    bool stop_resets = are_calories_under_threshold(env, context, stats, THRESHOLD);
                    if (stop_resets){
                        // Reset one additional time without rolling to reset position
                        route_reset_terrakion(env, context, stats, false);
                        route_check_terrakion(env, context, stats);
                        go_home(env.console, context);
                        reset_game_from_home(env, env.console, context);
                    }else{
                        route_reset_terrakion(env, context, stats);
                    }
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
                    stats.errors++;
                    env.update_stats();
                    env.console.overlay().add_log("Error Found. Reset Game", COLOR_RED);
                    go_home(env.console, context);
                    reset_game_from_home(env, env.console, context);
                }
                env.update_stats();
                if (stats.resets.load(std::memory_order_relaxed) % 100 == 0){
                    send_program_status_notification(env, NOTIFICATION_STATUS);
                }
            }
        },
        {{shiny_detector}}
    );

    shiny_sound_handler.process_pending(context);
    go_home(env.console, context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
