/*  LGPE Legendary Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Joycon.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "PokemonLGPE/Inference/Battles/PokemonLGPE_BattleArrowDetector.h"
#include "PokemonLGPE/Inference/Sounds/PokemonLGPE_ShinySoundDetector.h"
#include "PokemonLGPE/Programs/PokemonLGPE_GameEntry.h"
#include "PokemonLGPE_LegendaryReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

LegendaryReset_Descriptor::LegendaryReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLGPE:LegendaryReset",
        Pokemon::STRING_POKEMON + " LGPE", "Legendary Reset",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLGPE/LegendaryReset.md",
        "Shiny hunt Legendary Pokemon by resetting the game.",
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {ControllerFeature::NintendoSwitch_RightJoycon},
        FasterIfTickPrecise::NOT_FASTER
    )
{}

struct LegendaryReset_Descriptor::Stats : public StatsTracker{
    Stats()
        : resets(m_stats["Resets"])
        , shinies(m_stats["Shinies"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shinies");
    }
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
};
std::unique_ptr<StatsTracker> LegendaryReset_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

LegendaryReset::LegendaryReset()
    : TARGET(
        "<b>Target:</b><br>",
        {
            {Target::mewtwo, "mewtwo", "Mewtwo, Articuno, Zapdos, Moltres"},
            {Target::snorlax, "snorlax", "Snorlax"},
            //{Target::snorlax2, "snorlax2", "Snorlax (w/Fuji)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Target::mewtwo
    )
    , GO_HOME_WHEN_DONE(true)
    , NOTIFICATION_SHINY(
        "Shiny Found",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_SHINY,
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(TARGET);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

bool LegendaryReset::run_battle(SingleSwitchProgramEnvironment& env, JoyconContext& context){
    float shiny_coefficient = 1.0;
    ShinySoundDetector shiny_detector(env.logger(), [&](float error_coefficient) -> bool{
        shiny_coefficient = error_coefficient;
        return true;
    });
    BattleArrowWatcher battle_started(COLOR_YELLOW);

    env.log("Starting battle.");
    switch (TARGET) {
    case Target::mewtwo:
        pbf_mash_button(context, BUTTON_A, 5000ms); //untested
        break;
    case Target::snorlax:
        pbf_mash_button(context, BUTTON_A, 15000ms);
        break;
    case Target::snorlax2:
        pbf_mash_button(context, BUTTON_A, 25000ms); //can't test, not really worth it
        break;
    }
    context.wait_for_all_requests();

    int res = run_until<JoyconContext>(
        env.console, context,
        [&](JoyconContext& context) {
            int ret = wait_until(
                env.console, context,
                std::chrono::seconds(90),
                {{battle_started}}
            );
            if (ret == 0) {
                env.log("HP boxes detected.");
            } else {
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "run_battle(): Did not detect battle start.",
                    env.console
                );
            }
            context.wait_for_all_requests();
        },
        {{shiny_detector}}
    );
    shiny_detector.throw_if_no_sound();
    if (res == 0){
        env.log("Shiny detected!");
        return true;
    }
    env.log("Shiny not found.");

    return false;
}

void LegendaryReset::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    JoyconContext context(scope, env.console.controller<JoyconController>());
    assert_16_9_720p_min(env.logger(), env.console);
    LegendaryReset_Descriptor::Stats& stats = env.current_stats<LegendaryReset_Descriptor::Stats>();

    /*
    Setup:
    Stand in front of encounter. (And save)
    Start the program in game.
    Your lead must not be shiny.

    Settings:
    Text Speed fast
    Skip cutscene option?

    Mewtwo, Articuno, Zapdos, Moltres, Snorlax, Electrode(?)
    Don't do it on the first Snorlax, otherwise have to sit through fuji tutorial
    */

    while (true) {
        bool encounter_battle = run_battle(env, context);
        if (encounter_battle) {
            stats.shinies++;
            env.update_stats();
            send_program_notification(env, NOTIFICATION_SHINY, COLOR_YELLOW, "Shiny found!", {}, "", env.console.video().snapshot(), true);
            break;
        }
        env.log("No shiny found. Resetting game.");
        send_program_status_notification(
            env, NOTIFICATION_STATUS_UPDATE,
            "No shiny found. Resetting game."
        );

        //Reset game
        pbf_press_button(context, BUTTON_HOME, 200ms, 2000ms);
        reset_game_from_home(env, env.console, context, 3000ms);
        context.wait_for_all_requests();

        stats.resets++;
        env.update_stats();
    }

    if (GO_HOME_WHEN_DONE) {
        pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}
