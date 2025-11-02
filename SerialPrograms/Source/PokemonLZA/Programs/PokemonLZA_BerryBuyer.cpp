/*  Berry Buyer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA_BerryBuyer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;

BerryBuyer_Descriptor::BerryBuyer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:BerryBuyer",
        STRING_POKEMON + " LZA", "Berry Buyer",
        "Programs/PokemonLZA/BerryBuyer.html",
        "Buy EV reducing berries from stall.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

class BerryBuyer_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : purchases(m_stats["Purchases"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Purchases");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& purchases;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> BerryBuyer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


BerryBuyer::BerryBuyer()
    : BERRY_TYPE(
        "<b>Berry to Purchase:",
        {
            {BerryType::POMEG,  "pomeg",  "Pomeg (HP)"},
            {BerryType::KELPSY, "kelpsy", "Kelpsy (Attack)"},
            {BerryType::QUALOT, "qualot", "Qualot (Defence)"},
            {BerryType::HONDEW, "hondew", "Hondew (Special Attack)"},
            {BerryType::GREPA,  "grepa",  "Grepa (Special Defence)"},
            {BerryType::TAMATO, "tamato", "Tamato (Speed)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        BerryType::POMEG
    )
    , NUM_PURCHASE(
        "<b>Number to Purchase:</b><br>The number of berries you want to purchase.",
        LockMode::LOCK_WHILE_RUNNING,
        100, 1, 999
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(BERRY_TYPE);
    PA_ADD_OPTION(NUM_PURCHASE);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void BerryBuyer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    BerryBuyer_Descriptor::Stats& stats = env.current_stats<BerryBuyer_Descriptor::Stats>();
    assert_16_9_720p_min(env.logger(), env.console);
    while (true) {
        context.wait_for_all_requests();

        ButtonWatcher buttonA(
            COLOR_RED,
            ButtonType::ButtonA,
            {0.1, 0.1, 0.8, 0.8},
            &env.console.overlay()
        );
        SelectionArrowWatcher select(
            COLOR_YELLOW, &env.console.overlay(),
            SelectionArrowType::RIGHT,
            {0.715, 0.235, 0.045, 0.080}
        );
        SelectionArrowWatcher confirm(
            COLOR_YELLOW, &env.console.overlay(),
            SelectionArrowType::RIGHT,
            {0.715, 0.600, 0.045, 0.080}
        );
        FlatWhiteDialogWatcher dialog(COLOR_RED, &env.console.overlay());

        int ret = wait_until(
            env.console, context,
            30000ms,
                {
                    buttonA,
                    select,
                    confirm,
                    dialog,
                }
            );
        context.wait_for(100ms);

        switch (ret){
            case 0:
                env.log("Detected A button.");
                pbf_press_button(context, BUTTON_A, 160ms, 80ms);
                continue;

            case 1:
                env.log("Detected berry selection screen.");
                switch(BERRY_TYPE){
                    case BerryType::HONDEW:
                        pbf_press_dpad(context, DPAD_DOWN, 160ms, 80ms);
                    case BerryType::QUALOT:
                        pbf_press_dpad(context, DPAD_DOWN, 160ms, 80ms);
                    case BerryType::KELPSY:
                        pbf_press_dpad(context, DPAD_DOWN, 160ms, 80ms);
                    case BerryType::POMEG:
                        break;
                    case BerryType::GREPA:
                        pbf_press_dpad(context, DPAD_UP,   160ms, 80ms);
                    case BerryType::TAMATO:
                        pbf_press_dpad(context, DPAD_UP,   160ms, 80ms);
                        pbf_press_dpad(context, DPAD_UP,   160ms, 80ms);
                        break;
                }
                pbf_press_button(context, BUTTON_A, 160ms, 80ms);
                continue;

            case 2:
                env.log("Detected purchase confirm screen.");
                pbf_press_button(context, BUTTON_A, 160ms, 80ms);
                stats.purchases++;
                env.update_stats();

                if (stats.purchases == NUM_PURCHASE) {
                    pbf_mash_button(context, BUTTON_B, 5000ms);
                    GO_HOME_WHEN_DONE.run_end_of_program(context);
                    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
                    return;
                }

                continue;

            case 3:
                env.log("Detected white dialog.");
                pbf_press_button(context, BUTTON_A, 160ms, 80ms);
                continue;

            default:
                stats.errors++;
                env.update_stats();
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "No recognized state after 30 seconds.",
                    env.console
                );
        }
    }
}


}
}
}

