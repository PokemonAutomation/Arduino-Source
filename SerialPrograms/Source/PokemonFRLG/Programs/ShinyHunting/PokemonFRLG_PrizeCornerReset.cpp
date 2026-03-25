/*  Prize Corner Reset
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
#include "Pokemon/Pokemon_Strings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_PrizeSelectDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_StartMenuDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_ShinySymbolDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_SelectionArrowDetector.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_PrizeCornerReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

PrizeCornerReset_Descriptor::PrizeCornerReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:PrizeCornerReset",
        Pokemon::STRING_POKEMON + " FRLG", "Prize Corner Reset",
        "Programs/PokemonFRLG/PrizeCornerReset.html",
        "Redeem and soft reset for a shiny Game Corner prize.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct PrizeCornerReset_Descriptor::Stats : public StatsTracker{
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
std::unique_ptr<StatsTracker> PrizeCornerReset_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

PrizeCornerReset::PrizeCornerReset()
    : SLOT(
        "<b>Slot:</b><br>Position of the prize in the selection dialog.",
        {
            {0, "slot0", "Slot 1 (Abra)"},
            {1, "slot1", "Slot 2 (Clefairy)"},
            {2, "slot2", "Slot 3 (FR: Dratini / LG: Pinsir)"},
            {3, "slot3", "Slot 4 (FR: Scyther / LG: Dratini)"},
            {4, "slot4", "Slot 5 (Porygon)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        0
    )
    , NUM_REDEEM(
        "<b>Number of redemptions:</b><br>How many times to redeem a prize per reset. "
        "Make sure your party has enough room, and that you have enough coins for multiple redemptions."
        "<br> ex. When buying 3 Dratini (2,800 * 3 = 8,400 coins) in FireRed, have a party of 3, with 3 open slots. "
        "This cannot be done in LeafGreen, as Dratini costs more, so the max there would be 2 Dratini (4,600 * 2 = 9,200 coins) and your party size would be 4, with 2 open slots.",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1, 5
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
    PA_ADD_OPTION(SLOT);
    PA_ADD_OPTION(NUM_REDEEM);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void PrizeCornerReset::obtain_prize(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    PrizeCornerReset_Descriptor::Stats& stats = env.current_stats<PrizeCornerReset_Descriptor::Stats>();

    for (int attempts = 0;; attempts++){
        PrizeSelectWatcher prize_dialog(COLOR_RED);

        //Only 1 line to press through in english, not sure about other languages?
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                env.log("Talking to booth.");
                pbf_press_button(context, BUTTON_A, 320ms, 640ms);
                pbf_press_button(context, BUTTON_B, 320ms, 1680ms);
                pbf_press_button(context, BUTTON_B, 320ms, 1680ms);
                pbf_press_button(context, BUTTON_B, 320ms, 1680ms);
            },
            { prize_dialog }
        );
        context.wait_for_all_requests();
        if (ret == 0){
            break;
        }

        stats.errors++;
        env.update_stats();
        if (attempts >= 5){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "obtain_prize(): Unable to open prize menu after 5 attempts.",
                env.console
            );
        }

        env.log("Unable to find prize menu... Retrying...", COLOR_RED);

        pbf_mash_button(context, BUTTON_B, 5000ms);
    }

    //Select prize slot
    env.log("Selecting prize.");
    for (uint16_t c = 0; c < (uint16_t)SLOT.current_value(); c++){
        pbf_press_dpad(context, DPAD_DOWN, 320ms, 320ms);
    }
    context.wait_for_all_requests();

    //Select prize
    pbf_press_button(context, BUTTON_A, 320ms, 640ms);
    pbf_press_button(context, BUTTON_A, 320ms, 640ms);

    //Exit dialog
    pbf_mash_button(context, BUTTON_B, 2000ms);
    context.wait_for_all_requests();
}

void PrizeCornerReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    PrizeCornerReset_Descriptor::Stats& stats = env.current_stats<PrizeCornerReset_Descriptor::Stats>();

    home_black_border_check(env.console, context);

    /*
    * Settings: Text Speed fast. Default borders.
    * Setup: Have a party of 5. Stand in front of the prize redemption. Save game. Move cursor back to top.
    * Make sure you can afford the prize.
    */

    bool shiny_found = false;

    while (!shiny_found){
        for (uint16_t i = 0; i < NUM_REDEEM; i++){
            env.log("Obtaining prize.");
            obtain_prize(env, context);
        }
        stats.errors += open_slot_six(env.console, context);
        env.update_stats();

        env.log("Checking prizes.");
        for (uint16_t i = 0; i < NUM_REDEEM; i++){
            VideoSnapshot screen = env.console.video().snapshot();

            ShinySymbolDetector shiny_checker(COLOR_YELLOW);
            bool check = shiny_checker.read(env.console.logger(), screen);

            if (check){
                env.log("Shiny found!");
                stats.shinies++;
                env.update_stats();
                send_program_notification(
                    env,
                    NOTIFICATION_SHINY,
                    COLOR_YELLOW,
                    "Shiny found!",
                    {}, "",
                    screen,
                    true
                );
                if (TAKE_VIDEO){
                    pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 0ms);
                }
                shiny_found = true;
            }else{
                env.log("Prize is not shiny.");
            }

            if(i < NUM_REDEEM - 1) {
                //Check the next pokemon
                pbf_press_dpad(context, DPAD_UP, 320ms, 320ms);
                pbf_wait(context, 1000ms);
                context.wait_for_all_requests();
            }
        }

        if (!shiny_found){
            env.log("Out of Pokemon to check.");
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
    }

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}

