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
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Pokemon/Pokemon_Strings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_PrizeSelectDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_StartMenuDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_ShinySymbolDetector.h"
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
        ProgramControllerClass::StandardController_RequiresPrecision,
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
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void PrizeCornerReset::obtain_prize(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    PrizeCornerReset_Descriptor::Stats& stats = env.current_stats<PrizeCornerReset_Descriptor::Stats>();

    env.log("Talking to booth.");
    pbf_press_button(context, BUTTON_A, 320ms, 640ms);

    PrizeSelectWatcher prize_dialog(COLOR_RED);

    //Only 1 line to press through in english, not sure about other languages?
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context) {
            for (int i = 0; i < 5; i++) {
                pbf_press_button(context, BUTTON_B, 320ms, 320ms);
                pbf_wait(context, 600ms); //Don't go too fast, have to let the box pop up
                context.wait_for_all_requests();
            }
        },
        { prize_dialog }
    );
    context.wait_for_all_requests();
    if (ret < 0){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "obtain_prize(): Unable to find prize menu.",
            env.console
        );
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

    /*
    * Settings: Text Speed fast. Default borders.
    * Setup: Have a party of 5. Stand in front of the prize redemption. Save game. Move cursor back to top.
    * Make sure you can afford the prize.
    */

    bool shiny_found = false;

    while (!shiny_found) {
        obtain_prize(env, context);
        open_slot_six(env.console, context);

        VideoSnapshot screen = env.console.video().snapshot();

        ShinySymbolDetector shiny_checker(COLOR_YELLOW);
        shiny_found = shiny_checker.read(env.console.logger(), screen);

        if (shiny_found) {
            env.log("Shiny found!");
            stats.shinies++;
            send_program_notification(env, NOTIFICATION_SHINY, COLOR_YELLOW, "Shiny found!", {}, "", screen, true);
            break;
        } else {
            env.log("Prize is not shiny.");
            env.log("Soft resetting.");
            send_program_status_notification(
                env, NOTIFICATION_STATUS_UPDATE,
                "Soft resetting."
            );
            soft_reset(env.program_info(), env.console, context);
            stats.resets++;
            context.wait_for_all_requests();
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

