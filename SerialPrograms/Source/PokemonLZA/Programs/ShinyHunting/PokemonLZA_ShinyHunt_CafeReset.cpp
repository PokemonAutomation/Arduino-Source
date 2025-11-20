/*  Shiny Hunt - Cafe Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA_ShinyHunt_CafeReset.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {

using namespace Pokemon;


ShinyHunt_CafeReset_Descriptor::ShinyHunt_CafeReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:ShinyHunt-CafeReset", STRING_POKEMON + " LZA",
        "Cafe Reset",
        "Programs/PokemonLZA/ShinyHunt-CafeReset.html",
        "Shiny hunt by repeatedly fast trveling to the cafe to reset nearby " + STRING_POKEMON + " spawns.",
        ProgramControllerClass::StandardController_NoRestrictions, FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS, {}
    )
{}

class ShinyHunt_CafeReset_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : resets(m_stats["Resets"])
        , shinies(m_stats["Shiny Sounds"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shiny Sounds");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> ShinyHunt_CafeReset_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


ShinyHunt_CafeReset::ShinyHunt_CafeReset()
    : SHINY_DETECTED("Shiny Detected", "", "2000 ms", ShinySoundDetectedAction::NOTIFY_ON_FIRST_ONLY)
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
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void ShinyHunt_CafeReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    // Mash button B to let Switch register the controller
    pbf_mash_button(context, BUTTON_B, 500ms);

    ShinyHunt_CafeReset_Descriptor::Stats& stats = env.current_stats<ShinyHunt_CafeReset_Descriptor::Stats>();

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

    bool to_zoom_to_max = true;
    run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            while (true){
                context.wait_for_all_requests();
                shiny_sound_handler.process_pending(context);

                // Open map
                bool can_fast_travel = open_map(env.console, context, to_zoom_to_max);
                to_zoom_to_max = false;
                if (!can_fast_travel){
                    stats.errors++;
                    env.update_stats();
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "CafeReset: Cannot open map for fast travel.",
                        env.console
                    );
                }

                // Move map cursor upwards a little bit
                pbf_move_left_joystick(context, 128, 64, 100ms, 200ms);

                // Fly from map to reset spawns
                FastTravelState travel_status = fly_from_map(env.console, context);
                if (travel_status != FastTravelState::SUCCESS){
                    stats.errors++;
                    env.update_stats();
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "CafeReset: Cannot fast travel after moving map cursor.",
                        env.console
                    );
                }

                stats.resets++;
                env.update_stats();
                if (stats.resets.load(std::memory_order_relaxed) % 10 == 0){
                    send_program_status_notification(env, NOTIFICATION_STATUS);
                }
            } // end while
        },
        {{shiny_detector}}
    );

    //  Shiny sound detected and user requested stopping the program when
    //  detected shiny sound.
    shiny_sound_handler.process_pending(context);

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
