/*  Legendary Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_LegendaryReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

LegendaryReset_Descriptor::LegendaryReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:LegendaryReset",
        Pokemon::STRING_POKEMON + " FRLG", "Legendary Reset",
        "Programs/PokemonFRLG/LegendaryReset.html",
        "Shiny hunt legendary Pokemon using soft resets.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct LegendaryReset_Descriptor::Stats : public StatsTracker{
    Stats()
        : resets(m_stats["Resets"])
        , shinies(m_stats["Shinies"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> LegendaryReset_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

LegendaryReset::LegendaryReset()
    : TARGET(
        "<b>Target:</b><br>",
        {
            {Target::press_a, "press_a", "Press A: Articuno, Zapdos, Moltres, Mewtwo, Deoxys, Lugia, Electrode"},
            {Target::walk_up, "walk_up", "Walk Up: Ho-Oh"},
            {Target::snorlax, "snorlax", "Snorlax"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Target::press_a
    )
    , TAKE_VIDEO("<b>Take Video:</b><br>Record a video when the shiny is found.", LockMode::UNLOCK_WHILE_RUNNING, true)
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
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_STATIC(SHINY_REQUIRES_AUDIO);
    PA_ADD_OPTION(TARGET);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void LegendaryReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    LegendaryReset_Descriptor::Stats& stats = env.current_stats<LegendaryReset_Descriptor::Stats>();

    home_black_border_check(env.console, context);

    /*
    * Settings: Text Speed fast. Default borders. Audio required.
    * Setup: Stand in front of target. Save the game.
    * Lead can be shiny, reset is before lead is even sent out.
    * This is the same as RSE resets.
    * For deoxys solve the puzzle first.
    */

    while (true){
        switch (TARGET){
        case Target::press_a:
            env.log("Target: Press A.");
            //Talk to target
            pbf_press_button(context, BUTTON_A, 320ms, 320ms);
            break;
        case Target::walk_up:
            env.log("Target: Walk Up.");
            //Step forward to start the encounter.
            pbf_press_dpad(context, DPAD_UP, 320ms, 400ms);
            break;
        case Target::snorlax:
            env.log("Target: Snorlax.");
            //Use Pokeflute
            pbf_mash_button(context, BUTTON_A, 3000ms);
            //Wait a bit for the sound effect before B mash starts
            pbf_wait(context, 3000ms);
            context.wait_for_all_requests();
            break;
        }

        //Mash B until black screen detected but not over (entered battle)
        BlackScreenWatcher battle_entered(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                //Long mash + wait for cases like Deoxys
                pbf_mash_button(context, BUTTON_B, 10000ms);
                pbf_wait(context, 10000ms);
                context.wait_for_all_requests();
            },
            {battle_entered}
        );
        context.wait_for_all_requests();
        if (ret != 0){
            stats.errors++;
            env.update_stats();
            env.log("Failed to enter battle.", COLOR_RED);
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to enter battle.",
                env.console
            );
        }else{
            env.log("Battle started.");
        }

        //handle_encounter will wait for "POKEMON appeared!"
        bool legendary_shiny = handle_encounter(env.console, context, false);
        if (legendary_shiny){
            stats.shinies++;
            env.update_stats();
            send_program_notification(
                env,
                NOTIFICATION_SHINY,
                COLOR_YELLOW,
                "Shiny found!",
                {}, "",
                env.console.video().snapshot(),
                true
            );
            if (TAKE_VIDEO){
                pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 0ms);
            }
            break;
        }

        //No shiny found
        env.log("Soft resetting.");
        send_program_status_notification(
            env, NOTIFICATION_STATUS_UPDATE,
            "Soft resetting."
        );

        stats.errors += soft_reset(env.console, context);
        stats.resets++;
        env.update_stats();
        context.wait_for_all_requests();
    }

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}

