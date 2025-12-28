/*  Shiny Hunt - Hyperspace Legendary
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
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
        : resets(m_stats["Resets"])
        , rounds(m_stats["Rounds"])
        , shinies(m_stats["Shiny Sounds"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Rounds");
        m_display_order.emplace_back("Shiny Sounds");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& rounds;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> ShinyHunt_HyperspaceLegendary_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


ShinyHunt_HyperspaceLegendary::ShinyHunt_HyperspaceLegendary()
    : SHINY_DETECTED("Shiny Detected", "", "2000 ms", ShinySoundDetectedAction::STOP_PROGRAM)
    , LEGENDARY("<b>Hunt Route:</b>",
        {
            // {Legendary::LATIAS, "latias", "Latias"},
            // {Legendary::LATIOS, "latios", "Latios"},
            // {Legendary::COBALION, "cobalion", "Cobalion"},
            {Legendary::TERRAKION, "terrakion", "Terrakion"},
            {Legendary::VIRIZION,  "virizion",  "Virizion"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Legendary::VIRIZION
    )
    , MAX_ROUNDS(
        "<b>Max Rounds:</b><br>Max number of spawn attempts. Set to zero to have no max round limit. "
        "Make sure to leave enough time to catch found shinies."
        "<br>Cal. per sec: 1 Star: 1 Cal./s, 2 Star: 1.6 Cal./s, 3 Star: 3.5 Cal./s, 4 Star: 7.5 Cal./s, 5 Star: 10 Cal./s.",
        LockMode::UNLOCK_WHILE_RUNNING,
        100, 0 // default, min
    )
    , MIN_CALORIE_REMAINING(
        "<b>Minimum Cal. allowed:</b><br>The program will stop if the Calorie number is at or below this value."
        "<br>NOTE: the more star the hyperspace has the faster Calorie burns! Pick a minimum Calorie value that gives you enough time to catch shinies."
        "<br>Cal. per sec: 1 Star: 1 Cal./s, 2 Star: 1.6 Cal./s, 3 Star: 3.5 Cal./s, 4 Star: 7.5 Cal./s, 5 Star: 10 Cal./s",
        LockMode::UNLOCK_WHILE_RUNNING,
        600, 0, 9999 // default, min, max
    )
    , SAVE_ON_START(
        "<b>Save on Start:</b><br>Save the game when starting the program. Each cycle will start at the calorie count at the time of the save",
        LockMode::LOCK_WHILE_RUNNING,
        true) // default
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
    PA_ADD_OPTION(MAX_ROUNDS);
    PA_ADD_OPTION(MIN_CALORIE_REMAINING);
    PA_ADD_OPTION(SAVE_ON_START);
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}

namespace {

class LegendaryRoute {
public:
    virtual ~LegendaryRoute() = default;

    // The route used to reset and respawn the legendary
    virtual void reset(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        ShinyHunt_HyperspaceLegendary_Descriptor::Stats& stats,
        SimpleIntegerOption<uint16_t>& MIN_CALORIE_REMAINING){

        const uint16_t min_calorie = MIN_CALORIE_REMAINING;

        HyperspaceCalorieLimitWatcher calorie_watcher(env.logger(), min_calorie);
        const int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                while (true){
                    reset_route(env, context, stats);

                    stats.resets++;
                    env.update_stats();

                    uint16_t calorie_number = calorie_watcher.calorie_number();
                    const std::string log_msg = std::format("Calorie: {}/{}", calorie_number, min_calorie);
                    env.add_overlay_log(log_msg);
                    env.log(log_msg);
                }
            },
            {{calorie_watcher}}
        );
        if (ret < 0){
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                std::string(typeid(*this).name()) + " reset(): Error during reset loop.",
                env.console
            );
        }
        env.log("min calorie reached");
    }

    // Unique route used to reset each respective legendary, to be implemented by subclasses
    virtual void reset_route(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        ShinyHunt_HyperspaceLegendary_Descriptor::Stats& stats){

        return;
    }

    // The route used to check if the legendary is shiny after resetting
    virtual bool check(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        ShinyHunt_HyperspaceLegendary_Descriptor::Stats& stats){

        return false; // For legendaries that do not have checking implemented
    }

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
};

class TerrakionRoute : public LegendaryRoute {
public:
    void reset_route(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        ShinyHunt_HyperspaceLegendary_Descriptor::Stats& stats
    ) override {
        context.wait_for_all_requests();

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
    }

    bool check(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        ShinyHunt_HyperspaceLegendary_Descriptor::Stats& stats
    ) override {
        context.wait_for_all_requests();

        // Use warp pads to reset position
        detect_warp_pad(env, context, stats);
        pbf_press_button(context, BUTTON_A, 160ms, 80ms);
        detect_warp_pad(env, context, stats);
        pbf_press_button(context, BUTTON_A, 160ms, 80ms);
        detect_warp_pad(env, context, stats);

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

        go_home(env.console, context);
        reset_game_from_home(env, env.console, context);

        return false;
    }
};

class VirizionRoute : public LegendaryRoute {
public:
    void reset_route(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        ShinyHunt_HyperspaceLegendary_Descriptor::Stats& stats
    ) override {
        context.wait_for_all_requests();
        // running forward
        ssf_press_button(context, BUTTON_B, 0ms, 8800ms, 0ms);
        // Add 30 ms to avoid any drift using the balustrade
        pbf_move_left_joystick(context, {0, +1}, 4430ms, 0ms);
        // run back
        pbf_move_left_joystick(context, {0, -1}, 4400ms, 0ms);
        pbf_wait(context, 100ms);
    }
};

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
            env, env.console,
            stats.shinies,
            error_coefficient
        );
    });

    std::unique_ptr<LegendaryRoute> legendary_route;
    switch(LEGENDARY) {
        case Legendary::TERRAKION:
            legendary_route = std::make_unique<TerrakionRoute>();
            break;
        case Legendary::VIRIZION:
            legendary_route = std::make_unique<VirizionRoute>();
            break;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "legendary route not implemented",
                env.console
            );
    }

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

                bool should_stop = false;
                try{
                    legendary_route->reset(env, context, stats, MIN_CALORIE_REMAINING);
                    context.wait_for_all_requests();

                    should_stop = legendary_route->check(env, context, stats);
                    context.wait_for_all_requests();

                    stats.rounds++;
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
                    env.console.overlay().add_log("Error Found. Reset Game", COLOR_RED);
                    go_home(env.console, context);
                    reset_game_from_home(env, env.console, context);
                }
                env.update_stats();
                if (stats.rounds.load(std::memory_order_relaxed) % 10 == 0){
                    send_program_status_notification(env, NOTIFICATION_STATUS);
                }
                if (should_stop){
                    break;
                }
                if (MAX_ROUNDS > 0 && stats.rounds >= MAX_ROUNDS){
                    env.log(std::format("Reached round limit {}", static_cast<uint64_t>(MAX_ROUNDS)));
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
