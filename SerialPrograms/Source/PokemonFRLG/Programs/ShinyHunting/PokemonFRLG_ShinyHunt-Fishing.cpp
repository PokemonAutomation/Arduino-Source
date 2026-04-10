/*  Shiny Hunt - Fishing
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Program Overview:
 *  This program automates fishing encounters in Pokemon FireRed/LeafGreen
 *  and stops when a shiny Pokemon is detected.
 *
 *  Core Features:
 *  - Adaptive timing adjusts A-button press timing to maximize encounter rate
 *  - Fail-safe prevents soft-lock if dialog detection stalls
 *  - Automatic flee from non-shiny encounters
 *  - Initial autosave to protect progress
 *  - Error tracking for debugging stability issues
 *
 *  Assumptions:
 *  - Fishing rod is registered to the SELECT button
 *  - Player is facing fishable water
 *  - Lead Pokemon can always flee (Smoke Ball recommended)
 *  - Text speed is set to FAST
 */
#include <chrono>
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_BattleDialogs.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_SelectionArrowDetector.h"
#include "PokemonFRLG/Inference/Sounds/PokemonFRLG_ShinySoundDetector.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG/Programs/PokemonFRLG_StartMenuNavigation.h"
#include "PokemonFRLG_ShinyHunt-Fishing.h"

using namespace std::chrono_literals;

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonFRLG {


/*
===============================================================================
Descriptor
Defines program metadata and statistics tracking.
===============================================================================
*/
ShinyHuntFishing_Descriptor::ShinyHuntFishing_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:ShinyHuntFishing",
        Pokemon::STRING_POKEMON + " FRLG",
        "Shiny Hunt - Fishing",
        "Programs/PokemonFRLG/ShinyHunt-Fishing.html",
        "Automated fishing shiny hunt.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

/*Tracks runtime statistics displayed in UI.*/
struct ShinyHuntFishing_Descriptor::Stats : public StatsTracker {
    Stats()
        : encounters(m_stats["Encounters"])
        , shinies(m_stats["Shinies"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Encounters");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Errors");
    }
    std::atomic<uint64_t>& encounters;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker>
ShinyHuntFishing_Descriptor::make_stats() const {return std::make_unique<Stats>();}

/*
===============================================================================
Constructor
Defines user configurable options.
===============================================================================
*/
ShinyHuntFishing::ShinyHuntFishing() : SingleSwitchProgramInstance()

    // Adaptive delay before pressing A after casting rod.
    // Adjusted dynamically to maximize encounter success.
    , BITE_WAIT_MS(
        "<b>Bite Wait (ms)</b><br>"
        "Adaptive delay before pressing A.",
        LockMode::UNLOCK_WHILE_RUNNING,
        3000,
        2400,
        4200
    )

    // Return to HOME menu after shiny is found.
    , GO_HOME_WHEN_DONE(true)
    // Notification triggered when shiny detected.
    , NOTIFICATION_SHINY("Shiny found", true, true, ImageAttachmentMode::JPG, {"Notifs", "Showcase"})
    , NOTIFICATIONS({
        &NOTIFICATION_SHINY,
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(BITE_WAIT_MS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

/*
===============================================================================
Program Logic
===============================================================================
*/

void ShinyHuntFishing::program(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
) {
    auto& stats =
        env.current_stats<ShinyHuntFishing_Descriptor::Stats>();

    /*
    ---------------------------------------------------------------------------
    Ensure game is properly detected and scaled.
    Prevents capture card resolution mismatch issues.
    ---------------------------------------------------------------------------
    */
    home_black_border_check(env.console, context);

    /*
    Program setup requirements:
    - Player must be facing fishable water
    - Fishing rod must be registered
    - Text speed must be FAST
    */
    env.log("FRLG Fishing shiny hunt started.");

    /*
    Ensure we begin in overworld state.
    Clears any open dialog boxes.
    */
    pbf_mash_button(context, BUTTON_B, 1500ms);

    /*
    Perform autosave at program start.
    Protects progress if crash occurs later.
    */
    env.log("Performing initial autosave.");
    save_game_to_overworld(env.console, context);

    /*Return to overworld after saving.*/
    pbf_mash_button(context, BUTTON_B, 1000ms);

    /*
    ---------------------------------------------------------------------------
    Detector initialization
    These watchers monitor game state changes.
    ---------------------------------------------------------------------------
    */
    AdvanceWhiteDialogWatcher advance_dialog(COLOR_RED);
    WhiteDialogWatcher white_dialog(COLOR_RED);
    BlackScreenWatcher battle_entered(COLOR_RED);
    BattleDialogWatcher battle_dialog(COLOR_RED);
    BattleMenuWatcher battle_menu(COLOR_RED);

    /*
    ---------------------------------------------------------------------------
    Adaptive timing variables
    Adjusts A press timing to match bite window.
    ---------------------------------------------------------------------------
    */
    int bite_wait_ms = BITE_WAIT_MS;

    const int min_wait = 2400;
    const int max_wait = 4200;
    const int adjustment_step = 40;

    int failed_attempts = 0;

    /*
    ===========================================================================
    Main Fishing Loop
    ===========================================================================
    */
    while (true) {
        /*
        Cast fishing rod.
        Rod must be registered to SELECT.
        */
        pbf_press_button(context, BUTTON_MINUS, 20ms, 1000ms);

        /*
        Wait for one of the following:
        0 -> dialog with red arrow ("Oh! A bite!")
        1 -> white dialog box appears
        2 -> battle transition begins
        If timeout occurs, run_until returns negative.
        */
        int ret =
            wait_until(
                env.console,
                context,
                std::chrono::milliseconds(bite_wait_ms),
                {
                    advance_dialog,
                    white_dialog,
                    battle_entered
                }
            );

        /*
        Advance dialog text as quickly as possible.
        Prevents missing encounter trigger window.
        */
        if (ret == 0) {
            env.log("Advance dialog detected (red arrow).");
            pbf_mash_button(context, BUTTON_B, 800ms);
        } else if (ret == 1) {
            env.log("White dialog detected.");
            pbf_mash_button(context, BUTTON_B, 800ms);
        } else if (ret == 2) {
            env.log("Battle transition detected.");
        } else {
            env.log("No bite detected.");
            continue;
        }

        /*
        Confirm battle has started.
        Multiple detectors used to improve reliability:
        - black screen fade
        - teal dialog box
        - FIGHT menu appearance
        */
        int battle_ret =
            wait_until(
                env.console,
                context,
                4000ms,
                {
                    battle_entered,
                    battle_dialog,
                    battle_menu
                }
            );

        /*
        -----------------------------------------------------------------------
        Dialog fail-safe
        Sometimes dialog detection may stall.
        If dialog persists too long, mash A to force progression.
        -----------------------------------------------------------------------
        */
        constexpr auto DIALOG_FAILSAFE_TIMEOUT =
            std::chrono::seconds(8);
        bool dialog_seen = false;
        WallClock dialog_start;
        if (ret == 0 || ret == 1) {
            dialog_seen = true;
            dialog_start = current_time();
        }
        if (
            dialog_seen &&
            current_time() - dialog_start >
            DIALOG_FAILSAFE_TIMEOUT
        ) {
            env.log("Dialog persisted too long. "
                "Triggering fail-safe."
            );
            pbf_mash_button(context, BUTTON_A, 1500ms);

            /*Attempt to recover battle state.*/
            battle_ret =
                wait_until(
                    env.console,
                    context,
                    2000ms,
                    {
                        battle_entered,
                        battle_dialog,
                        battle_menu
                    }
                );

            /*Count error only if fail-safe did not recover battle.*/
            if (battle_ret < 0) {
                env.log("Fail-safe recovery failed.");
                stats.errors++;
                env.update_stats();
            }
        }

        /*
        -----------------------------------------------------------------------
        No encounter triggered
        Adjust timing upward to improve bite sync.
        -----------------------------------------------------------------------
        */
        if (battle_ret < 0) {
            failed_attempts++;

            /*
            Increase delay gradually.
            Prevents over-adjusting due to random variance.
            */
            if (failed_attempts >= 3) {
                bite_wait_ms += adjustment_step;
                if (bite_wait_ms > max_wait) {
                    bite_wait_ms = max_wait;
                }
                BITE_WAIT_MS.set(bite_wait_ms);
                failed_attempts = 0;
            }
            continue;
        }

        /*
        -----------------------------------------------------------------------
        Encounter triggered successfully
        -----------------------------------------------------------------------
        */
        stats.encounters++;
        env.update_stats();
        failed_attempts = 0;

        /*
        Slightly reduce delay to improve efficiency.
        */
        bite_wait_ms -= adjustment_step;
        if (bite_wait_ms < min_wait) {
            bite_wait_ms = min_wait;
        }
        BITE_WAIT_MS.set(bite_wait_ms);

        /*
        Detect shiny animation/audio.
        handle_encounter waits for Pokemon to appear.
        */
        bool shiny = handle_encounter(env.console, context, true);

        /*
        -----------------------------------------------------------------------
        Shiny detected
        -----------------------------------------------------------------------
        */
        if (shiny) {
            stats.shinies++;
            env.update_stats();
            send_program_notification(
                env,
                NOTIFICATION_SHINY,
                COLOR_YELLOW,
                "Shiny found!",
                {},
                "",
                env.console.video().snapshot(),
                true
            );
            break;
        }
        /*
        -----------------------------------------------------------------------
        Non-shiny encounter
        Assumes fleeing always succeeds.
        -----------------------------------------------------------------------
        */
        flee_battle(env.console, context);
        context.wait_for_all_requests();
    }

    /*Return to HOME menu after shiny found.*/
    if (GO_HOME_WHEN_DONE) {
        pbf_press_button(
            context,
            BUTTON_HOME,
            200ms,
            1000ms
        );
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH
    );
}
}  // namespace PokemonFRLG
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation