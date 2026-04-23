/*  Item Duplication
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Color.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonFRLG/Inference/PokemonFRLG_SelectionArrowDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_PartyHeldItemDetector.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_ItemDuplication.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

ItemDuplication_Descriptor::ItemDuplication_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:ItemDuplication",
        Pokemon::STRING_POKEMON + " FRLG", "Item Duplication",
        "Programs/PokemonFRLG/ItemDuplication.html",
        "Duplicate items using the Retro Mail glitch.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct ItemDuplication_Descriptor::Stats : public StatsTracker {
    public:
    Stats()
        : duplications(m_stats["Items Duplicated"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Items Duplicated");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& duplications;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> ItemDuplication_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

ItemDuplication::ItemDuplication()
    : STOP_AFTER_CURRENT("Duplication")
    , NUM_DUPLICATIONS(
        "<b>Number of Duplications:</b><br>"
        "Zero will run until 'Stop after Current Duplication' is pressed or the program is manually stopped.",
        LockMode::UNLOCK_WHILE_RUNNING,
        100,
        0
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(STOP_AFTER_CURRENT);

    PA_ADD_OPTION(NUM_DUPLICATIONS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);

    PA_ADD_OPTION(NOTIFICATIONS);
}

void ItemDuplication::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

    home_black_border_check(env.console, context);

    ItemDuplication_Descriptor::Stats& stats = env.current_stats<ItemDuplication_Descriptor::Stats>();
    DeferredStopButtonOption::ResetOnExit reset_on_exit(STOP_AFTER_CURRENT);

    ImageFloatBox mail_arrow_box{ 0.3670769231, 0.06373076639, 0.028, 0.077 };
    ImageFloatBox mail_confirmation_arrow_box{ 0.7289230769, 0.4604230588, 0.029, 0.076 };
    SelectionArrowWatcher mail_selection_arrow(COLOR_BLUE, &env.console.overlay(), mail_arrow_box);
    PartyHeldItemWatcher farfetchd_held_item(COLOR_BLUE, &env.console.overlay(), PartyHeldItemSlot::SLOT_1);
    AdvanceWhiteDialogWatcher item_description(COLOR_BLUE);
    SelectionArrowWatcher confirmation_arrow(COLOR_BLUE, &env.console.overlay(), SelectionArrowPositionConfirmationMenu::YES);
    SelectionArrowWatcher mail_confirmation_arrow(COLOR_BLUE, &env.console.overlay(), mail_confirmation_arrow_box);

    while (true){
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
        if (NUM_DUPLICATIONS != 0 && stats.duplications >= NUM_DUPLICATIONS){
            break;
        }

        // Detect retro mail in bag
        int ret = wait_until(
            env.console, context,
            std::chrono::seconds(4),
            { mail_selection_arrow }
        );

        if (ret < 0) {
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "program(): Unable to detect selection arrow for Retro Mail. Please ensure Retro Mail is the top slot.",
                env.console
            );
        }

        pbf_press_button(context, BUTTON_A, 200ms, 100ms);
        pbf_press_dpad(context, DPAD_DOWN, 200ms, 100ms);
        pbf_press_button(context, BUTTON_A, 200ms, 100ms);

        // Detect held item to be duplicated
        int ret2 = wait_until(
            env.console, context,
            std::chrono::seconds(4),
            { farfetchd_held_item }
        );

        if (ret2 < 0) {
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "program(): Unable to detect farfetchd held item. Please ensure farfetchd is holding an item to duplicate.",
                env.console
            );
        }

        pbf_press_button(context, BUTTON_A, 200ms, 300ms);

        // Item description text box
        int ret3 = wait_until(
            env.console, context,
            std::chrono::seconds(4),
            { item_description }
        );

        if (ret3 < 0) {
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "program(): Unable to detect item description.",
                env.console
            );
        }

        pbf_press_button(context, BUTTON_A, 200ms, 100ms);

        // Confirmation prompt to swap item to farfetchd
        int ret4 = wait_until(
            env.console, context,
            std::chrono::seconds(4),
            { confirmation_arrow }
        );

        if (ret4 < 0) {
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "program(): Unable to detect confirmation prompt.",
                env.console
            );
        }
        pbf_press_button(context, BUTTON_A, 200ms, 100ms);
        pbf_wait(context, 500ms); // Quick load screen
        context.wait_for_all_requests();

        // Dectect "???" screen?

        pbf_press_dpad(context, DPAD_UP, 200ms, 100ms);
        pbf_press_dpad(context, DPAD_LEFT, 200ms, 100ms);
        pbf_press_button(context, BUTTON_A, 200ms, 100ms);

        // Confirmation prompt in "???" screen
        int ret5 = wait_until(
            env.console, context,
            std::chrono::seconds(4),
            { mail_confirmation_arrow }
        );

        if (ret5 < 0) {
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "program(): Unable to detect confirmation prompt on mail screen.",
                env.console
            );
        }
        pbf_press_button(context, BUTTON_A, 200ms, 100ms);
        pbf_wait(context, 300ms); // Quick load screen
        context.wait_for_all_requests();

        stats.duplications++;
        env.update_stats();

        if (STOP_AFTER_CURRENT.should_stop()){
            break;
        }
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}


}
}
}
