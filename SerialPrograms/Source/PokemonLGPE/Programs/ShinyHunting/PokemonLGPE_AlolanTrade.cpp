/*  LGPE Alolan Trade
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
#include "PokemonLGPE_AlolanTrade.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

AlolanTrade_Descriptor::AlolanTrade_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLGPE:AlolanTrade",
        Pokemon::STRING_POKEMON + " LGPE", "Alolan Trade",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLGPE/AlolanTrade.md",
        "Shiny hunt Alolan forms by trading in-game.",
        ProgramControllerClass::SpecializedController,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct AlolanTrade_Descriptor::Stats : public StatsTracker{
    Stats()
        : trades(m_stats["Trades"])
        , resets(m_stats["Resets"])
        , shinies(m_stats["Shinies"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Trades");
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& trades;
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
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

void AlolanTrade::run_trade(SingleSwitchProgramEnvironment& env, JoyconContext& context){
    AlolanTrade_Descriptor::Stats& stats = env.current_stats<AlolanTrade_Descriptor::Stats>();
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
        stats.errors++;
        env.update_stats();
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
        stats.errors++;
        env.update_stats();
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

    //Summary will appear the first time you trade in a session(?)
    //Sometimes it appears anyway, don't know what determines it
    //Exit menu and dialog.
    pbf_mash_button(context, BUTTON_B, 3000ms);
    context.wait_for_all_requests();
}

void AlolanTrade::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    JoyconContext context(scope, env.console.controller<RightJoycon>());
    assert_16_9_720p_min(env.logger(), env.console);
    AlolanTrade_Descriptor::Stats& stats = env.current_stats<AlolanTrade_Descriptor::Stats>();

    /*
    WARNING: JOYCON TEST PROGRAM. Not well tested. Bare minimum in general.

    Right joycon required for home button (this means no on-switch screenshots).
    Also don't remap any of the buttons in the switch button mapping settings.

    Preconditions:
    DO NOT have any Pokemon you want to keep in your boxes. Move them out to Home first.
    Favoriting a Pokemon does not prevent it from being traded.
    ?This must not be your first time doing the trade? (I've done all the trades, so I can't check first time trade behavior.)

    Setup:
    Catch the Kanto variant of the target. Put this number in NUM_TRADES.
    Stand in front of trade NPC.
    Save the game.
    Start the program in-game.
    
    Future additions?:
    skip favorited pokemon - they have a symbol to indicate fav status (imo still risky to do, just move them out)
    detect when all pokemon traded - trading screen will open but text will appear in the center
    "you don't have any pokemon your trading partner wants"
    detect dialog box, detect yes/no confirm box
    actual enter game and start screen detectors
    menu detectors, etc.
    get rid of all this blindly mashing A in general...so need everything really.
    */

    bool shiny_found = false;
    while (!shiny_found) {
        //Run trades
        for (uint16_t i = 0; i < NUM_TRADES; i++) {
            env.log("Running trade.");
            run_trade(env, context);

            stats.trades++;
            env.update_stats();
        }

        env.log("Done trading. Checking boxes.");
        send_program_status_notification(
            env, NOTIFICATION_STATUS_UPDATE,
            "Done trading. Checking boxes."
        );

        //To check pokemon in menu boxes
        //Open menu - always defaults to center (Party)
        //Menu:
        // --Play with Partner--(centered)
        //Pokedex - Bag - Party - Communicate - Save (these all have a colored line under when selected + an arrow to indicate)
        //(Press Y for options)

        //Make sure any extra dialog is closed
        pbf_mash_button(context, BUTTON_B, 3000ms);
        context.wait_for_all_requests();

        //Wait a bit.
        pbf_wait(context, 2500ms);
        context.wait_for_all_requests();

        //Open menu, open party, open boxes
        env.log("Opening boxes.");
        pbf_press_button(context, BUTTON_X, 200ms, 500ms);
        pbf_press_button(context, BUTTON_A, 200ms, 1500ms);
        pbf_press_button(context, BUTTON_Y, 200ms, 2000ms);
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

        //View summary - it takes a moment to load
        pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
        pbf_move_joystick(context, 128, 255, 100ms, 100ms);
        pbf_move_joystick(context, 128, 255, 100ms, 100ms);
        pbf_press_button(context, BUTTON_A, 200ms, 100ms);
        context.wait_for_all_requests();

        pbf_wait(context, 5000ms);
        context.wait_for_all_requests();

        //Now check for shinies. Check everything that was traded.
        env.log("Checking received Pokemon.");
        for (uint16_t i = 0; i < NUM_TRADES; i++) {
            VideoSnapshot screen = env.console.video().snapshot();
            ShinySymbolDetector shiny_checker(COLOR_YELLOW);
            bool check = shiny_checker.read(env.console.logger(), screen);

            if (check) {
                env.log("Shiny detected!");
                stats.shinies++;
                env.update_stats();
                send_program_notification(env, NOTIFICATION_SHINY, COLOR_YELLOW, "Shiny found!", {}, "", screen, true);
                shiny_found = true;

                //Back out to menu and favorite the shiny.
                env.log("Favoriting shiny.");
                pbf_press_button(context, BUTTON_B, 200ms, 5000ms);
                pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
                pbf_move_joystick(context, 128, 0, 100ms, 100ms);
                pbf_move_joystick(context, 128, 0, 100ms, 100ms);
                pbf_move_joystick(context, 128, 0, 100ms, 200ms);
                pbf_press_button(context, BUTTON_A, 200ms, 800ms);
                pbf_press_button(context, BUTTON_A, 200ms, 800ms);
                pbf_press_button(context, BUTTON_B, 200ms, 800ms);

                //Go into summary again
                env.log("Navigating back into summary.");
                pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
                pbf_move_joystick(context, 128, 255, 100ms, 100ms);
                pbf_move_joystick(context, 128, 255, 100ms, 100ms);
                pbf_press_button(context, BUTTON_A, 200ms, 100ms);
                context.wait_for_all_requests();
                pbf_wait(context, 5000ms);
                context.wait_for_all_requests();
            }
            else {
                env.log("Not shiny.");
            }

            //Move left, check next.
            pbf_move_joystick(context, 0, 128, 100ms, 100ms);
            pbf_press_button(context, BUTTON_X, 0ms, 1000ms);
            context.wait_for_all_requests();
        }
        
        if (!shiny_found) {
            env.log("Out of Pokemon to trade and no shiny found. Resetting game.");
            send_program_status_notification(
                env, NOTIFICATION_STATUS_UPDATE,
                "Out of Pokemon to trade and no shiny found. Resetting game."
            );


            reset_game_from_game(env, env.console, context, &stats.errors, 3000ms);
            stats.resets++;
            env.update_stats();
        }
    }

    if (GO_HOME_WHEN_DONE) {
        pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}
