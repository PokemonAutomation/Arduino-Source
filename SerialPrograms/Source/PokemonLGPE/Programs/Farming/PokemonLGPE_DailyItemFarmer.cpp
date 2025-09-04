/*  LGPE Daily Item Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
//#include "CommonFramework/VideoPipeline/VideoFeed.h"
//#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Joycon.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "Pokemon/Pokemon_Strings.h"
//#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "PokemonLGPE/Commands/PokemonLGPE_DateSpam.h"
//#include "PokemonLGPE/Inference/PokemonLGPE_ShinySymbolDetector.h"
//#include "PokemonLGPE/Programs/PokemonLGPE_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonLGPE_DailyItemFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

DailyItemFarmer_Descriptor::DailyItemFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLGPE:DailyItemFarmer",
        Pokemon::STRING_POKEMON + " LGPE", "Daily Item Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLGPE/DailyItemFarmer.md",
        "Farm daily item respawns (ex. fossils) by date-skipping.",
        ProgramControllerClass::SpecializedController,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct DailyItemFarmer_Descriptor::Stats : public StatsTracker{
    Stats()
        : skips(m_stats["Skips"])
    {
        m_display_order.emplace_back("Skips");
    }
    std::atomic<uint64_t>& skips;
};
std::unique_ptr<StatsTracker> DailyItemFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

DailyItemFarmer::DailyItemFarmer()
    : ATTEMPTS(
        "<b>Number of attempts:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        30, 1
    )
    , LINK_CODE(
        "<b>Link Code:</b><br>The link code used when matching for a trade/battle. This only needs to be changed when running multiple LGPE date-skip programs at the same time.",
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
    , FIX_TIME_WHEN_DONE(
        "<b>Fix Time when Done:</b><br>Fix the time after the program finishes.",
        LockMode::UNLOCK_WHILE_RUNNING, false
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(ATTEMPTS);
    PA_ADD_OPTION(LINK_CODE);
    PA_ADD_OPTION(FIX_TIME_WHEN_DONE);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void DailyItemFarmer::start_local_trade(SingleSwitchProgramEnvironment& env, JoyconContext& context){
    env.log("Starting local trade.");
    //Open Menu -> Communication -> Nearby player -> Local Trade
    pbf_press_button(context, BUTTON_X, 200ms, 500ms);
    pbf_move_joystick(context, 255, 128, 100ms, 100ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
    pbf_wait(context, 1000ms); //Black screen
    context.wait_for_all_requests();
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
    pbf_press_button(context, BUTTON_A, 200ms, 100ms);
    pbf_press_button(context, BUTTON_A, 200ms, 100ms);
    pbf_press_button(context, BUTTON_A, 200ms, 100ms);
    pbf_wait(context, 1000ms); //let search start
    context.wait_for_all_requests();
}

void DailyItemFarmer::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    JoyconContext context(scope, env.console.controller<RightJoycon>());
    assert_16_9_720p_min(env.logger(), env.console);
    DailyItemFarmer_Descriptor::Stats& stats = env.current_stats<DailyItemFarmer_Descriptor::Stats>();

    /* Stand in front of the fossil spawn near Mewtwo.
    *  Use a repel to keep wild encounters away.
    *  Start program in-game.
    *  100% daily spawn. Only works near Mewtwo.
    *  Other cave item spawns are tied to steps taken.
    *  Should work for other hidden daily items, game corner, mt moon moonstones, etc.
    */

    uint8_t year = MAX_YEAR;

    //Roll the date back before doing anything else.
    start_local_trade(env, context);
    pbf_press_button(context, BUTTON_HOME, 160ms, 1000ms);
    home_to_date_time(env.console, context, true);
    env.log("Rolling date back.");
    roll_date_backward_N(context, MAX_YEAR);
    year = 0;
    pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
    pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
    pbf_mash_button(context, BUTTON_B, 5000ms);
    context.wait_for_all_requests();

    env.log("Starting pickup loop.");
    for (uint32_t count = 0; count < ATTEMPTS; count++) {
        env.log("Pick up item.");
        pbf_mash_button(context, BUTTON_A, 4000ms);
        context.wait_for_all_requests();

        start_local_trade(env, context);

        //Dateskip
        pbf_press_button(context, BUTTON_HOME, 160ms, 1000ms);
        home_to_date_time(env.console, context, true);
        if (year >= MAX_YEAR){
            env.log("Rolling date back.");
            roll_date_backward_N(context, MAX_YEAR);
            year = 0;
        }else{
            env.log("Rolling date forward.");
            roll_date_forward_1(context);
            year++;
        }
        pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);

        //Re-enter game and close out link menu
        pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
        pbf_mash_button(context, BUTTON_B, 5000ms);
        context.wait_for_all_requests();

        stats.skips++;
        env.update_stats();
    }

    if (FIX_TIME_WHEN_DONE){
        go_home(env.console, context);
        home_to_date_time(env.console, context, false);
        pbf_press_button(context, BUTTON_A, 50ms, 500ms);
        pbf_press_button(context, BUTTON_A, 50ms, 500ms);
        pbf_wait(context, 100ms);
        context.wait_for_all_requests();
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
