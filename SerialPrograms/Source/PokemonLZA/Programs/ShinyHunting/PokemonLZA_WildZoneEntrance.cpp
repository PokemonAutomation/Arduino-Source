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
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA_WildZoneEntrance.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {

using namespace Pokemon;


ShinyHunt_WildZoneEntrance_Descriptor::ShinyHunt_WildZoneEntrance_Descriptor()
    : SingleSwitchProgramDescriptor("PokemonLZA:ShinyHunt-WildZoneEntrance", STRING_POKEMON + " LZA",
                                    "Shiny Hunt - Wild Zone Entrance",
                                    "Programs/PokemonLZA/ShinyHunt-WildZoneEntrance.html",
                                    "Shiny hunt by repeatedly entering Wild Zone from its entrance.",
                                    ProgramControllerClass::StandardController_NoRestrictions, FeedbackType::REQUIRED,
                                    AllowCommandsWhenRunning::DISABLE_COMMANDS, {}) {}
class ShinyHunt_WildZoneEntrance_Descriptor::Stats : public StatsTracker {
public:
    Stats() : resets(m_stats["Wild Zone"]), shinies(m_stats["Shiny Sounds"]), errors(m_stats["Errors"]) {
        m_display_order.emplace_back("Wild Zone");
        m_display_order.emplace_back("Shiny Sounds");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);

        m_aliases["Shinies"] = "Shiny Sounds";
        m_aliases["Shinies Detected"] = "Shiny Sounds";
    }

    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> ShinyHunt_WildZoneEntrance_Descriptor::make_stats() const {
    return std::unique_ptr<StatsTracker>(new Stats());
}


ShinyHunt_WildZoneEntrance::ShinyHunt_WildZoneEntrance()
    : WALK_IN_ZONE("<b>WALK IN ZONE:</b><br>Walk this long in the zone after passing through the gate.",
                   LockMode::UNLOCK_WHILE_RUNNING, "500 ms"),
      SHINY_DETECTED("Shiny Detected", "", "2000 ms", ShinySoundDetectedAction::NOTIFY_ON_FIRST_ONLY),
      NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600)),
      NOTIFICATIONS({
          &NOTIFICATION_STATUS,
          &SHINY_DETECTED.NOTIFICATIONS,
          &NOTIFICATION_PROGRAM_FINISH,
          &NOTIFICATION_ERROR_RECOVERABLE,
          &NOTIFICATION_ERROR_FATAL,
      }) {
    PA_ADD_STATIC(SHINY_REQUIRES_AUDIO);
    PA_ADD_OPTION(WALK_IN_ZONE);
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void run_to_gate(ConsoleHandle& console, ProControllerContext& context) {
    ButtonWatcher buttonA(COLOR_RED, ButtonType::ButtonA, {0, 0, 1, 1}, &console.overlay());
    int ret = run_until<ProControllerContext>(console, context,
                                              [](ProControllerContext& context) {
                                                  for (int c = 0; c < 10; c++) {
                                                      pbf_move_left_joystick(context, 128, 0, 800ms, 200ms);
                                                  }
                                              },
                                              {{buttonA}});

    switch (ret) {
        case 0:
            break;
        default:
            OperationFailedException::fire(ErrorReport::SEND_ERROR_REPORT,
                                           "run_to_gate(): Unable to detect entrance after 10 seconds.", console);
    }
}

void enter_wild_zone_entrance(ConsoleHandle& console, ProControllerContext& context, Milliseconds walk_in_zone) {
    BlackScreenOverWatcher black_screen(COLOR_BLUE);
    int ret = run_until<ProControllerContext>(console, context,
                                              [&](ProControllerContext& context) {
                                                  pbf_mash_button(context, BUTTON_B, 200ms);  // dismiss menu if any
                                                  run_to_gate(console, context);
                                                  pbf_mash_button(context, BUTTON_A, 2000ms);
                                                  pbf_move_left_joystick(context, 128, 0, walk_in_zone, 200ms);
                                                  context.wait_for_all_requests();
                                                  pbf_press_button(context, BUTTON_PLUS, 100ms, 100ms);  // open map
                                              },
                                              {{black_screen}});
    if (ret == 0) {
        console.log("[WildZoneEntrance] Detected day/night change after entering.");
        context.wait_for(std::chrono::milliseconds(2000));
        pbf_mash_button(context, BUTTON_B, 200ms);             // dismiss menu if any
        pbf_press_button(context, BUTTON_PLUS, 100ms, 100ms);  // open map again
    }
    pbf_mash_button(context, BUTTON_A, 800ms);  // teleporting or just mashing button
    pbf_mash_button(context, BUTTON_B, 200ms);  // in case need to dismiss map
    context.wait_for_all_requests();
    context.wait_for(std::chrono::milliseconds(4000));  // TODO: differ NS1 wait time from NS2
}

void ShinyHunt_WildZoneEntrance::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    ShinyHunt_WildZoneEntrance_Descriptor::Stats& stats =
        env.current_stats<ShinyHunt_WildZoneEntrance_Descriptor::Stats>();

    uint8_t shiny_count = 0;

    {  // first run with shiny detection
        float shiny_coefficient = 1.0;
        PokemonLA::ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool {
            //  Warning: This callback will be run from a different thread than this function.
            shiny_count++;
            stats.shinies++;
            env.update_stats();
            shiny_coefficient = error_coefficient;
            return true;
        });

        run_until<ProControllerContext>(env.console, context,
                                        [&](ProControllerContext& context) {
                                            while (true) {
                                                send_program_status_notification(env, NOTIFICATION_STATUS);
                                                stats.resets++;
                                                enter_wild_zone_entrance(env.console, context, WALK_IN_ZONE);
                                                env.update_stats();
                                            }
                                        },
                                        {{shiny_detector}});

        context.wait_for(std::chrono::milliseconds(1000));

        // when shiny sound is detected, it's most likely happened inside the zone
        // now try to reset position
        pbf_mash_button(context, BUTTON_B, 200ms);             // dismiss menu if any
        pbf_press_button(context, BUTTON_PLUS, 100ms, 100ms);  // open map
        pbf_mash_button(context, BUTTON_A, 600ms);             // teleporting or just mashing button
        pbf_mash_button(context, BUTTON_B, 200ms);             // in case need to dismiss map

        SHINY_DETECTED.on_shiny_sound(env, env.console, context, shiny_count, shiny_coefficient);
    }

    // continue with shiny detection if pointless
    if (SHINY_DETECTED.ACTION != ShinySoundDetectedAction::STOP_PROGRAM) {
        while (true) {
            send_program_status_notification(env, NOTIFICATION_STATUS);
            stats.resets++;
            enter_wild_zone_entrance(env.console, context, WALK_IN_ZONE);
            env.update_stats();
        }
    }

    go_home(env.console, context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
