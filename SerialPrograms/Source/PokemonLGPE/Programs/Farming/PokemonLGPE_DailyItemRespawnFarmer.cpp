/*  LGPE Daily Item Respawn Farmer
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
#include "PokemonLGPE/Inference/PokemonLGPE_ShinySymbolDetector.h"
#include "PokemonLGPE/Programs/PokemonLGPE_GameEntry.h"
#include "PokemonLGPE_DailyItemRespawnFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

DailyItemRespawnFarmer_Descriptor::DailyItemRespawnFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLGPE:DailyItemRespawnFarmer",
        Pokemon::STRING_POKEMON + " LGPE", "Daily Item Respawn Farmer",
        "",
        "Farm daily item respawns (ex. fossils) by date-skipping.",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {ControllerFeature::NintendoSwitch_RightJoycon},
        FasterIfTickPrecise::NOT_FASTER
    )
{}

struct DailyItemRespawnFarmer_Descriptor::Stats : public StatsTracker{
    Stats()
        : skips(m_stats["Skips"])
        , resets(m_stats["Resets"])
    {
        m_display_order.emplace_back("Skips");
        m_display_order.emplace_back("Resets");
    }
    std::atomic<uint64_t>& skips;
    std::atomic<uint64_t>& resets;
};
std::unique_ptr<StatsTracker> DailyItemRespawnFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

DailyItemRespawnFarmer::DailyItemRespawnFarmer()
    : ATTEMPTS(
        "<b>Number of attempts:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        30, 1
    )
    , LINK_CODE(
        "<b>Link Code:</b><br>Only needed when running multiple LGPE date-skip programs at the same time. The link code used when matching for a trade/battle.",
        {   //Combinations of 3 different symbols is possible but 10 choices seems like enough.
            {LinkCode::Pikachu,     "pikachu",      "Pikachu"},
            {LinkCode::Eevee,       "eevee",        "Eevee"},
            {LinkCode::Bulbasaur,   "bulbasaur",    "Bulbasaur"},
            {LinkCode::Charmander,  "charmander",   "Charmander"},
            {LinkCode::Squirtle,    "squirtle",     "Squirtle"},
            {LinkCode::Pidgey,      "pidgey",       "Pidgey"},
            {LinkCode::Caterpie,    "caterpie",     "Caterpie"},
            {LinkCode::Rattata,     "rattata",      "Rattata"},
            {LinkCode::Jigglypuff,  "jigglypuff",   "Jigglypuff"},
            {LinkCode::Diglett,     "diglett",      "Diglett"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        LinkCode::Pikachu
    )
    , GO_HOME_WHEN_DONE(false)
    , FIX_TIME_WHEN_DONE(
        "<b>Fix Time when Done:</b><br>Fix the time after the program finishes.",
        LockMode::UNLOCK_WHILE_RUNNING, false
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(ATTEMPTS);
    PA_ADD_OPTION(LINK_CODE);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(FIX_TIME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void DailyItemRespawnFarmer::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    JoyconContext context(scope, env.console.controller<JoyconController>());
    assert_16_9_720p_min(env.logger(), env.console);
    DailyItemRespawnFarmer_Descriptor::Stats& stats = env.current_stats<DailyItemRespawnFarmer_Descriptor::Stats>();

    /* Stand in front of the fossil spawn near Mewtwo.
    *  Use a repel to keep wild encounters away.
    *  Start program in-game.
    *  100% daily spawn. Only works near Mewtwo.
    *  Other cave item spawns are tied to steps taken.
    *  Should work for other hidden daily items, game corner, mt moon moonstones, etc.
    */



    for (uint32_t count = 0; count < ATTEMPTS; count++) {
        //Pick up item
        pbf_mash_button(context, BUTTON_A, 3000ms);

        //Open Menu -> Communication -> Nearby player -> Local Trade
        pbf_press_button(context, BUTTON_X, 200ms, 500ms);
        pbf_move_joystick(context, 255, 128, 100ms, 100ms);
        pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
        pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
        pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
        pbf_press_button(context, BUTTON_A, 200ms, 1000ms);

        //Enter link code
        switch(LINK_CODE) {
        case LinkCode::Pikachu:
            break;
        case LinkCode::Eevee:
            pbf_move_joystick(context, 255, 128, 100ms, 100ms);
            break;
        case LinkCode::Bulbasaur:
            pbf_move_joystick(context, 255, 128, 100ms, 100ms);
            pbf_move_joystick(context, 255, 128, 100ms, 100ms);
            break;
        case LinkCode::Charmander:
            pbf_move_joystick(context, 0, 128, 100ms, 100ms);
            pbf_move_joystick(context, 0, 128, 100ms, 100ms);
            break;
        case LinkCode::Squirtle:
            pbf_move_joystick(context, 0, 128, 100ms, 100ms);
            break;
        case LinkCode::Pidgey:
            pbf_move_joystick(context, 128, 255, 100ms, 100ms);
            break;
        case LinkCode::Caterpie:
            pbf_move_joystick(context, 128, 255, 100ms, 100ms);
            pbf_move_joystick(context, 255, 128, 100ms, 100ms);
            break;
        case LinkCode::Rattata:
            pbf_move_joystick(context, 128, 255, 100ms, 100ms);
            pbf_move_joystick(context, 0, 128, 100ms, 100ms);
            pbf_move_joystick(context, 0, 128, 100ms, 100ms);
            break;
        case LinkCode::Jigglypuff:
            pbf_move_joystick(context, 128, 255, 100ms, 100ms);
            pbf_move_joystick(context, 0, 128, 100ms, 100ms);
            pbf_move_joystick(context, 0, 128, 100ms, 100ms);
            break;
        case LinkCode::Diglett:
            pbf_move_joystick(context, 128, 255, 100ms, 100ms);
            pbf_move_joystick(context, 0, 128, 100ms, 100ms);
            break;
        default:
            env.log("Invalid link code selection. Defaulting to Pikachu.");
            break;
        }
        //Select symbol three times, then enter link search
        pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
        pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
        pbf_press_button(context, BUTTON_A, 200ms, 1000ms);

        //Dateskip

        //Re-enter game

        //Close out link menu

        //stats.

        stats.skips++;
        env.update_stats();
    }

    if (FIX_TIME_WHEN_DONE){
        pbf_press_button(context, BUTTON_HOME, 80ms, GameSettings::instance().GAME_TO_HOME_DELAY1);
        home_to_date_time(context, false, false);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
        resume_game_from_home(env.console, context);
    }

    if (GO_HOME_WHEN_DONE) {
        pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}
