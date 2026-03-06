/*  Shiny Hunt - Overworld
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_ShinyHunt-Overworld.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

ShinyHuntOverworld_Descriptor::ShinyHuntOverworld_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:OverworldReset",
        Pokemon::STRING_POKEMON + " FRLG", "Shiny Hunt - Overworld",
        "Programs/PokemonFRLG/OverworldReset.html",
        "Shiny hunt Overworld Pokemon.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct ShinyHuntOverworld_Descriptor::Stats : public StatsTracker{
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
std::unique_ptr<StatsTracker> ShinyHuntOverworld_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

ShinyHuntOverworld::ShinyHuntOverworld()
    : TRIGGER_METHOD(
        "<b>Maneuver:</b><br>How to trigger an encounter",
        {
            {TriggerMethod::HORIZONTAL_NO_BIAS,     "horizontal-none",  "Move left/right. (no bias)"},
            {TriggerMethod::HORIZONTAL_BIAS_LEFT,   "horizontal-left",  "Move left/right. (bias left)"},
            {TriggerMethod::HORIZONTAL_BIAS_RIGHT,  "horizontal-right", "Move left/right. (bias right)"},
            {TriggerMethod::VERTICAL_NO_BIAS,       "vertical-none",    "Move up/down. (no bias)"},
            {TriggerMethod::VERTICAL_BIAS_UP,       "vertical-up",      "Move up/down. (bias up)"},
            {TriggerMethod::VERTICAL_BIAS_DOWN,     "vertical-down",    "Move up/down. (bias down)"}
        },
        LockMode::LOCK_WHILE_RUNNING,
        TriggerMethod::HORIZONTAL_NO_BIAS
        )
    , MOVE_DURATION0(
        "<b>Move Duration:</b><br>Move in each direction for this long before turning around.",
        LockMode::LOCK_WHILE_RUNNING,
        "1000 ms"
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
    PA_ADD_OPTION(TRIGGER_METHOD);
    PA_ADD_OPTION(MOVE_DURATION0);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void ShinyHuntOverworld::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ShinyHuntOverworld_Descriptor::Stats& stats = env.current_stats<ShinyHuntOverworld_Descriptor::Stats>();

    /*
    * Settings: Text Speed fast. Audio required.
    * Setup: Stand in grass.
    * Lead can't be shiny.
    */

    while (true){
        bool encounter_found = find_encounter(env, context);

        if (!encounter_found){
            stats.errors += 1;
            env.update_stats();
            break;
        }

        //handle_encounter will wait for "POKEMON appeared!"
        bool encounter_shiny = handle_encounter(env.console, context, true);
        if (encounter_shiny){
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

        flee_battle(env.console, context);
        context.wait_for_all_requests();

        stats.encounters++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
    }

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

bool ShinyHuntOverworld::find_encounter(SingleSwitchProgramEnvironment& env, ProControllerContext& context) const
{
    BlackScreenWatcher battle_entered(COLOR_RED);

    Milliseconds normal_duration = MOVE_DURATION0;
    Milliseconds biased_duration = MOVE_DURATION0.get() + 200ms;
    Milliseconds mash_duration = normal_duration - 64ms;

    int ret = 1;
    while (ret != 0){
        switch (TRIGGER_METHOD){
        case TriggerMethod::HORIZONTAL_NO_BIAS:
            ret = run_until<ProControllerContext>(
                env.console, context,
                [normal_duration, mash_duration](ProControllerContext& context) {
                    ssf_press_left_joystick(context, { -1, 0 }, 0ms, normal_duration);
                    ssf_mash1_button(context, BUTTON_B, mash_duration);
                    ssf_press_left_joystick(context, { +1, 0 }, 0ms, normal_duration);
                    ssf_mash1_button(context, BUTTON_B, mash_duration);
                },
                { battle_entered }
            );
            break;
        case TriggerMethod::HORIZONTAL_BIAS_LEFT:
            ret = run_until<ProControllerContext>(
                env.console, context,
                [normal_duration, mash_duration, biased_duration](ProControllerContext& context) {
                    ssf_press_left_joystick(context, { -1, 0 }, 0ms, biased_duration);
                    ssf_mash1_button(context, BUTTON_B, mash_duration);
                    ssf_press_left_joystick(context, { +1, 0 }, 0ms, normal_duration);
                    ssf_mash1_button(context, BUTTON_B, mash_duration);
                },
                { battle_entered }
            );
            break;
        case TriggerMethod::HORIZONTAL_BIAS_RIGHT:
            ret = run_until<ProControllerContext>(
                env.console, context,
                [normal_duration, mash_duration, biased_duration](ProControllerContext& context) {
                    ssf_press_left_joystick(context, { +1, 0 }, 0ms, biased_duration);
                    ssf_mash1_button(context, BUTTON_B, mash_duration);
                    ssf_press_left_joystick(context, { -1, 0 }, 0ms, normal_duration);
                    ssf_mash1_button(context, BUTTON_B, mash_duration);
                },
                { battle_entered }
            );
            break;
        case TriggerMethod::VERTICAL_NO_BIAS:
            ret = run_until<ProControllerContext>(
                env.console, context,
                [normal_duration, mash_duration](ProControllerContext& context) {
                    ssf_press_left_joystick(context, { 0, +1 }, 0ms, normal_duration);
                    ssf_mash1_button(context, BUTTON_B, mash_duration);
                    ssf_press_left_joystick(context, { 0, -1 }, 0ms, normal_duration);
                    ssf_mash1_button(context, BUTTON_B, mash_duration);
                },
                { battle_entered }
            );
            break;
        case TriggerMethod::VERTICAL_BIAS_UP:
            ret = run_until<ProControllerContext>(
                env.console, context,
                [normal_duration, mash_duration, biased_duration](ProControllerContext& context) {
                    ssf_press_left_joystick(context, { 0, +1 }, 0ms, biased_duration);
                    ssf_mash1_button(context, BUTTON_B, mash_duration);
                    ssf_press_left_joystick(context, { 0, -1 }, 0ms, normal_duration);
                    ssf_mash1_button(context, BUTTON_B, mash_duration);
                },
                { battle_entered }
            );
            break;
        case TriggerMethod::VERTICAL_BIAS_DOWN:
            ret = run_until<ProControllerContext>(
                env.console, context,
                [normal_duration, mash_duration, biased_duration](ProControllerContext& context) {
                    ssf_press_left_joystick(context, { 0, -1 }, 0ms, biased_duration);
                    ssf_mash1_button(context, BUTTON_B, mash_duration);
                    ssf_press_left_joystick(context, { 0, +1 }, 0ms, normal_duration);
                    ssf_mash1_button(context, BUTTON_B, mash_duration);
                },
                { battle_entered }
            );
            break;
        default:;
        }
    }

    return true;
}

}
}
}
