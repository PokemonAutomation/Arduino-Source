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
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/PokemonLZA_HyperspaceCalorieDetector.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA_ShinyHunt_HyperspaceLegendary.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {

using namespace Pokemon;


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
    : SHINY_DETECTED("Shiny Detected", "", "2000 ms", ShinySoundDetectedAction::NOTIFY_ON_FIRST_ONLY)
    , LEGENDARY("<b>Hunt Route:</b>",
        {
            {Legendary::VIRIZION,  "virizion",  "Virizion"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Legendary::VIRIZION
    )
    , MIN_CALORIE_REMAINING(
        "<b>Minimum Cal. allowed:</b><br>The program will stop if the Calorie number is at or below this value."
        "<br>NOTE: the more star the hyperspace has the faster Calorie burns! Pick a minimum Calorie value that gives you enough time to catch shinies."
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
    PA_ADD_OPTION(MIN_CALORIE_REMAINING);
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}

namespace {

bool hunt_virizion(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    ShinyHunt_HyperspaceLegendary_Descriptor::Stats& stats,
    SimpleIntegerOption<uint16_t>& MIN_CALORIE_REMAINING){

    const uint16_t min_calorie = MIN_CALORIE_REMAINING;

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
    
        int ret = wait_until(
            env.console, context, std::chrono::seconds(1), {calorie_watcher}
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "hunt_Virizion(): does not detect Calorie number after waiting for a second",
                env.console
            );
        }

        const uint16_t calorie_number = calorie_watcher.calorie_number();
        const std::string log_msg = std::format("Calorie: {}/{}", calorie_number, min_calorie);
        env.add_overlay_log(log_msg);
        env.log(log_msg);
        if (calorie_number <= min_calorie){
            env.log("min calorie reached");
            break;
        }
    }

    // We have done enough shuttle runs to refresh Virizion spawns.
    // Now run towards it to check shiny!
    
    // Push left joystick rightward to let the character face right
    pbf_move_left_joystick(context, {+1, 0}, 100ms, 0ms);
    // Roll once to leave the balcony area
    pbf_press_button(context, BUTTON_Y, 100ms, 1s);
    // Run downstairs towards the trash bin
    pbf_move_left_joystick(context, {0, +1}, Seconds(5), 0ms);

    return false;
}


} // namespace

void ShinyHunt_HyperspaceLegendary::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    // Mash button B to let Switch register the controller
    pbf_mash_button(context, BUTTON_B, 200ms);

    ShinyHunt_HyperspaceLegendary_Descriptor::Stats& stats = env.current_stats<ShinyHunt_HyperspaceLegendary_Descriptor::Stats>();

    ShinySoundHandler shiny_sound_handler(SHINY_DETECTED);

    PokemonLA::ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool {
        //  Warning: This callback will be run from a different thread than this function.
        stats.shinies++;
        env.update_stats();
        env.console.overlay().add_log("Shiny Sound Detected!", COLOR_YELLOW);

        return shiny_sound_handler.on_shiny_sound(
            env, env.console, stats.shinies, error_coefficient
        );
    });

    run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            while (true){
                context.wait_for_all_requests();
                shiny_sound_handler.process_pending(context);

                bool should_stop = false;
                if (LEGENDARY == Legendary::VIRIZION){
                    should_stop = hunt_virizion(env, context, stats, MIN_CALORIE_REMAINING);
                } else{
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "legendary hunt not implemented",
                        env.console
                    );
                }
                stats.game_resets++;
                env.update_stats();
                send_program_status_notification(env, NOTIFICATION_STATUS);

                if (should_stop){
                    break;
                }

            } // end while
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
