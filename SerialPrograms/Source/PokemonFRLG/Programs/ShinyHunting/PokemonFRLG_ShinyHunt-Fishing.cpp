/*  Shiny Hunt - Fishing
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"

#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"

#include "Pokemon/Pokemon_Strings.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_BattleDialogs.h"
#include "PokemonFRLG/Inference/PokemonFRLG_SelectionArrowDetector.h"
#include "PokemonFRLG/Inference/Sounds/PokemonFRLG_ShinySoundDetector.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"

#include <chrono>

#include "PokemonFRLG_ShinyHunt-Fishing.h"

/*TODO LIST
 * Based on reading the included .h files, I believe I can add the following functionality in the future.
 * -An option to automatically register a new rod.
 * -Audio shiny detection for redundancy
 * -add periodic saves every X events where events= ((Y missed casts)+(Z encounters) and X is defined by the user.
 * -The option for a user to select a ball and auto throw until caught or runs out of usable pokemon
 * ---This is a stretch goal I don't know if I will ever implement. It sounds cool and fun, but I doubt I have the skill.
 * */

using namespace std::chrono_literals;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

ShinyHuntFishing_Descriptor::
    ShinyHuntFishing_Descriptor()
    : SingleSwitchProgramDescriptor(
          "PokemonFRLG:ShinyHuntFishing",
          Pokemon::STRING_POKEMON + " FRLG", "Shiny Hunt - Fishing",
          "Programs/PokemonFRLG/ShinyHunt-Fishing.html",
          "Automated fishing shiny hunt.",
          ProgramControllerClass::StandardController_NoRestrictions,
          FeedbackType::REQUIRED,
          AllowCommandsWhenRunning::DISABLE_COMMANDS
          )
{}

struct ShinyHuntFishing_Descriptor::Stats : public StatsTracker{
    Stats()
        : encounters(m_stats["Encounters"])
        , shinies(m_stats["Shinies"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Encounters");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& encounters;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> ShinyHuntFishing_Descriptor::make_stats() const{
    return std::make_unique<Stats>();
}


ShinyHuntFishing::ShinyHuntFishing()
    : SingleSwitchProgramInstance()
    , BITE_WAIT_MS(
          "<b>Bite Wait (ms)</b><br>"
          "Adaptive delay before pressing A.",
          LockMode::UNLOCK_WHILE_RUNNING,
          3000,
          2400,
          4200
          )
    , GO_HOME_WHEN_DONE(true)

    , NOTIFICATION_SHINY(
          "Shiny found",
          true, true, ImageAttachmentMode::JPG,
          {"Notifs", "Showcase"}
          )

    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
          &NOTIFICATION_SHINY,
          &NOTIFICATION_STATUS_UPDATE,
          &NOTIFICATION_PROGRAM_FINISH
      })

{
    PA_ADD_OPTION(BITE_WAIT_MS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
    /*PA_ADD_STATIC(SHINY_REQUIRES_AUDIO);
    PA_ADD_OPTION(TRIGGER_METHOD);
    PA_ADD_OPTION(MOVE_DURATION0);
    PA_ADD_OPTION(TAKE_VIDEO);*/
}


void ShinyHuntFishing::program(SingleSwitchProgramEnvironment& env,ProControllerContext& context
    ){
    auto& stats = env.current_stats<ShinyHuntFishing_Descriptor::Stats>();

    home_black_border_check(env.console, context);
    /*
    * Settings: Text Speed fast.
    * Setup: Stand in facing fishable water.
    */

    env.log("FRLG Fishing shiny hunt started.");

    // ensure overworld state
    pbf_mash_button(context, BUTTON_B, 1500ms);

    WhiteDialogWatcher dialog(COLOR_RED);
    BlackScreenWatcher battle_entered(COLOR_RED);

    int bite_wait_ms = BITE_WAIT_MS;

    while (true){

        // cast rod (must already be registered)
        pbf_press_button(context, BUTTON_MINUS, 20ms, 1400ms);

        // wait for bite dialog or battle transition
        int ret =
            run_until<ProControllerContext>(
                env.console,
                context,
                [&](ProControllerContext& context){
                    pbf_wait(context, std::chrono::milliseconds(bite_wait_ms));
                },
                {
                    dialog,
                    battle_entered
                }
                );

        // if dialog detected → advance text quickly
        if (ret == 0){
            run_until<ProControllerContext>(
                env.console,
                context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_A, 1000ms);
                },
                { battle_entered }
                );
        }

        // wait for battle to actually begin
        int battle_ret =
            run_until<ProControllerContext>(
                env.console,
                context,
                [](ProControllerContext& context){
                    pbf_wait(context, 1500ms);
                },
                { battle_entered }
                );

        // no encounter triggered
        if (battle_ret != 0){
            continue;
        }

        // encounter triggered
        stats.encounters++;
        env.update_stats();

        // shiny detection
        bool shiny =
            handle_encounter(
                env.console,
                context,
                true
                );


        if (shiny){

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


        // flee non-shiny encounter
        flee_battle(env.console, context);

        context.wait_for_all_requests();
    }


    if (GO_HOME_WHEN_DONE){
        pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
    }


    send_program_finished_notification(
        env,
        NOTIFICATION_PROGRAM_FINISH
        );
}

}
}
}
