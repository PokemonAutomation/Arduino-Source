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
        LockMode::UNLOCK_WHILE_RUNNING,
        30
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
    //PA_ADD_OPTION(GO_HOME_WHEN_DONE);
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
    Also don't remap any of the buttons in the switch button mapping settings. Yet?

    Preconditions:
    DO NOT have any Pokemon you want to keep in your boxes. Move them out to Home first.
    Favoriting a Pokemon does not prevent it from being traded.
    This must not be your first time doing the trade. (I've done all the trades, can't check first time trade behavior.)

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

    //to check pokemon in menu boxes - not used
    //Open menu - always defaults to center (Party)
    /* Menu:
    Play with Partner
    Pokedex - Bag - Party - Communicate - Save
    (Press Y for options)

    sort boxes by recently caught and press left to get to most recent pokemon
    */

    /*
    pbf_press_button(context, BUTTON_A, 200ms, 2000ms);
    pbf_press_button(context, BUTTON_HOME, 200ms, 2000ms);
    pbf_move_joystick(context, 128, 0, 100ms, 100ms);
    pbf_move_joystick(context, 128, 0, 100ms, 100ms);
    pbf_move_joystick(context, 255, 128, 100ms, 100ms);
    pbf_move_joystick(context, 128, 0, 100ms, 100ms);
    pbf_press_button(context, BUTTON_X, 200ms, 2000ms);
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
            BlackScreenOverWatcher trade_completed(COLOR_RED);
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

            //After black screen fade is done, a summary will appear.
            //pbf_wait(context, 250);
            context.wait_for_all_requests();

            VideoSnapshot screen = env.console.video().snapshot();
            ShinySymbolDetector shiny_checker(COLOR_YELLOW);
            shiny_found = shiny_checker.read(env.console.logger(), screen);

            if (shiny_found) {
                env.log("Shiny detected!");
                stats.shinies++;
                send_program_status_notification(env, NOTIFICATION_SHINY, "Shiny found!", screen, true);
                break;
            }
            else {
                env.log("Not shiny.");
                stats.trades++;
            }
        }

        if (!shiny_found) {
            //Go to home, reset game
            //How to handle sideways joycons vs in-game? What if a set is paired?
            //Set as-is for now - I only have one ESP32, don't know how we're handling multiple joycons w/our usual home functions

            env.log("Out of Pokemon to trade. Resetting game.");
            send_program_status_notification(
                env, NOTIFICATION_STATUS_UPDATE,
                "Out of Pokemon to trade. Resetting game."
            );
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
