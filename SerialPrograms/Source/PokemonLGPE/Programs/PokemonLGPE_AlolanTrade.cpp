/*  LGPE Alolan Trade
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
#include "Pokemon/Pokemon_Strings.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "PokemonLGPE/Inference/PokemonLGPE_ShinySymbolDetector.h"
#include "PokemonLGPE_AlolanTrade.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

AlolanTrade_Descriptor::AlolanTrade_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLGPE:AlolanTrade",
        Pokemon::STRING_POKEMON + " LGPE", "Alolan Trade",
        "",
        "Shiny hunt Alolan forms by trading in-game.",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {ControllerFeature::NintendoSwitch_RightJoycon},
        FasterIfTickPrecise::NOT_FASTER
    )
{}

struct AlolanTrade_Descriptor::Stats : public StatsTracker{
    Stats()
        : trades(m_stats["Trades"])
        , resets(m_stats["Resets"])
        , shinies(m_stats["Shinies"])
    {
        m_display_order.emplace_back("Trades");
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shinies");
    }
    std::atomic<uint64_t>& trades;
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
};
std::unique_ptr<StatsTracker> AlolanTrade_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

AlolanTrade::AlolanTrade()
    : NUM_TRADES(
        "<b>Number of Pokemon to trade:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        30, 1
    )
    , GO_HOME_WHEN_DONE(false)
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
    PA_ADD_OPTION(NUM_TRADES);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void AlolanTrade::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    JoyconContext context(scope, env.console.controller<JoyconController>());
    assert_16_9_720p_min(env.logger(), env.console);
    AlolanTrade_Descriptor::Stats& stats = env.current_stats<AlolanTrade_Descriptor::Stats>();

    /*
    WARNING: JOYCON TEST PROGRAM. Not well tested. Minimum infra to get this running. Bare minimum in general.
    Use at your own risk, it won't skip update checks and the like.
    FLASH RIGHT JOYCON. YOU NEED RIGHT JOYCON. YOU NEED THE HOME BUTTON. (this means no on-switch screenshots)
    Also don't remap any of the buttons in the switch button mapping settings. Yet? Could use this to add Home and Screenshot.

    Preconditions:
    DO NOT have any Pokemon you want to keep in your boxes. Move them out to Home first.
    Favoriting a Pokemon does not prevent it from being traded.
    This must not be your first time doing the trade. (I've done all the trades, can't check first time trade behavior.)
    In your boxes, sort by ORDER CAUGHT

    Setup:
    Catch the Kanto variant of the target.
    Stand in front of trade NPC.
    Start the program in-game.
    
    Future additions?:
    skip favorited pokemon - they have a symbol to indicate fav status (imo still risky to do, just move them out)
    detect when all pokemon traded - trading screen will open but text will appear in the center
    "you don't have any pokemon your trading partner wants"
    detect dialog box, detect yes/no confirm box
    actual enter game and start screen detectors
    menu detectors, reset game from home, etc.
    get rid of all this blindly mashing A in general...so need everything really.
    */

    bool shiny_found = false;
    while (!shiny_found) {
        //Run trades
        for (uint16_t i = 0; i < NUM_TRADES; i++) {
            //TODO: This is messy, pull it all out. run_trade()?

            //Talk to NPC, say Yes, select Pokemon from box.
            BlackScreenOverWatcher trade_started(COLOR_RED);
            int ret = run_until<JoyconContext>(
                env.console, context,
                [](JoyconContext& context){
                    pbf_mash_button(context, BUTTON_A, 15000ms);
                },
                {trade_started}
            );
            context.wait_for_all_requests();
            if (ret != 0){
                env.log("Failed to start trade.", COLOR_RED);
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Failed to start trade.",
                    env.console
                );
            }
            else {
                env.log("Trade started.");
            }

            //Wait for trade to complete.
            BlackScreenOverWatcher trade_completed(COLOR_YELLOW);
            int ret2 = wait_until(
                env.console, context,
                std::chrono::seconds(120),
                {trade_completed}
            );
            context.wait_for_all_requests();
            if (ret2 != 0){
                env.log("Did not detect end of trade.", COLOR_RED);
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Did not detect end of trade.",
                    env.console
                );
            }
            else {
                env.log("Trade completed.");
            }

            //Summary will appear the first time you trade in a session(?) Close that as well.
            //Exit menu and dialog.
            pbf_mash_button(context, BUTTON_B, 3000ms);
            context.wait_for_all_requests();

            stats.trades++;
            env.update_stats();
        }

        //to check pokemon in menu boxes
        //Open menu - always defaults to center (Party)
        /* Menu:
        Play with Partner
        Pokedex - Bag - Party - Communicate - Save (these all have a colored line under when selected)
        (Press Y for options)

        sort boxes by recently caught and press left to get to most recent pokemon
        */

        //Wait a bit.
        pbf_press_button(context, BUTTON_X, 0ms, 2500ms);
        context.wait_for_all_requests();

        //Open menu, open party, open boxes
        env.log("Opening boxes.");
        pbf_press_button(context, BUTTON_X, 200ms, 500ms);
        pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
        pbf_press_button(context, BUTTON_Y, 200ms, 1500ms);
        context.wait_for_all_requests();

        //Sort by order caught
        env.log("Sorting by order caught.");
        pbf_press_button(context, BUTTON_Y, 200ms, 1000ms);
        pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
        pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
        context.wait_for_all_requests();

        //Press left to go to last (most recent) Pokemon
        env.log("Opening summary of most recent Pokemon.");
        pbf_move_joystick(context, 0, 128, 100ms, 100ms);
        context.wait_for_all_requests();

        //View summary - it takes a moment to load, wait is below
        pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
        pbf_move_joystick(context, 128, 255, 100ms, 100ms);
        pbf_move_joystick(context, 128, 255, 100ms, 100ms);
        pbf_press_button(context, BUTTON_A, 200ms, 100ms);
        context.wait_for_all_requests();

        //Wait.
        pbf_press_button(context, BUTTON_X, 0ms, 5000ms);
        context.wait_for_all_requests();

        //Now check for shinies. Check everything that was traded.
        for (uint16_t i = 0; i < NUM_TRADES; i++) {
            VideoSnapshot screen = env.console.video().snapshot();
            ShinySymbolDetector shiny_checker(COLOR_YELLOW);
            bool check = shiny_checker.read(env.console.logger(), screen);

            if (check) {
                env.log("Shiny detected!");
                stats.shinies++;
                env.update_stats();
                send_program_status_notification(env, NOTIFICATION_SHINY, "Shiny found!", screen, true);
                shiny_found = true;
            }
            else {
                env.log("Not shiny.");
            }

            //Move left, check next.
            pbf_move_joystick(context, 0, 128, 100ms, 100ms);
            pbf_press_button(context, BUTTON_X, 0ms, 2000ms);
            context.wait_for_all_requests();
        }

        if (!shiny_found) {
            //TODO? Check if home button even exists before attempting to reset.
            //This way, if on left joycon, stop the program and alert the user.

            env.log("Out of Pokemon to trade. Resetting game.");
            send_program_status_notification(
                env, NOTIFICATION_STATUS_UPDATE,
                "Out of Pokemon to trade. Resetting game."
            );

            //TODO: Need to make proper GameEntry eventually
            //Thankfully, Joycon is upright after going to home.
            //Go to home and close game
            pbf_press_button(context, BUTTON_HOME, 200ms, 3000ms);
            pbf_press_button(context, BUTTON_X, 200ms, 1000ms);
            pbf_press_button(context, BUTTON_A, 200ms, 1000ms);

            //Enter game from home
            //break;

            stats.resets++;
            env.update_stats();
        }
    }

    //GO_HOME_WHEN_DONE.run_end_of_program(context);
    if (GO_HOME_WHEN_DONE) {
        pbf_press_button(context, BUTTON_HOME, 200ms, 3000ms);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}
