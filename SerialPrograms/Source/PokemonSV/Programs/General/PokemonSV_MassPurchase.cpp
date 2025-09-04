/*  SV Mass Purchase
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV_MassPurchase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{
    using namespace Pokemon;

MassPurchase_Descriptor::MassPurchase_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:MassPurchase",
        STRING_POKEMON + " SV", "Mass Purchase",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/MassPurchase.md",
        "Purchase a specified amount of items from a shop.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct MassPurchase_Descriptor::Stats : public StatsTracker{
    Stats()
        : total_items(m_stats["Items Purchased"])
        , skip(m_stats["Skipped"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back(Stat("Items Purchased"));
        m_display_order.emplace_back(Stat("Skipped"));
        m_display_order.emplace_back(Stat("Errors"));
    }
    std::atomic<uint64_t>& total_items;
    std::atomic<uint64_t>& skip;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> MassPurchase_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

MassPurchase::MassPurchase()
    : ITEMS(
        "<b>Items to Buy:</b><br>The amount of Items to buy from the position of the cursor.",
        LockMode::LOCK_WHILE_RUNNING,
        50
    )
    , QUANTITY(
        "<b>Quantity to Buy:</b><br>The amount of each item to buy. If a clothing store, set to 1.",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1, 999
    )
    , GO_HOME_WHEN_DONE(false)
    , PAY_LP(
        "<b>Pay with LP:</b>",
        LockMode::UNLOCK_WHILE_RUNNING, false
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(ITEMS);
    PA_ADD_OPTION(QUANTITY);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(PAY_LP);
    PA_ADD_OPTION(NOTIFICATIONS);
}

bool MassPurchase::mass_purchase(ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context){
    MassPurchase_Descriptor::Stats& stats = env.current_stats<MassPurchase_Descriptor::Stats>();

    OverworldWatcher overworld(stream.logger(), COLOR_RED);
    AdvanceDialogWatcher dialog(COLOR_CYAN);
    context.wait_for_all_requests();
    int ret = wait_until(
        stream, context,
        std::chrono::seconds(2),
        {
            overworld,
            dialog,
        }
    );
    context.wait_for(std::chrono::milliseconds(50));

    switch (ret){
    case 0:
        env.log("Error - Stuck in Overworld");
        stats.errors++;
        throw_and_log<FatalProgramException>(
            stream.logger(), ErrorReport::SEND_ERROR_REPORT,
            "Stuck in Overworld.",
            stream
        );

    case 1:
        env.log("Detected full bag. Skipped Item");
        stats.skip++;
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
        return true;

    default:
        return false;
    }
};
    
bool MassPurchase::extra_items(ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context){
    MassPurchase_Descriptor::Stats& stats = env.current_stats<MassPurchase_Descriptor::Stats>();

    OverworldWatcher overworld(stream.logger(), COLOR_RED);
    AdvanceDialogWatcher dialog(COLOR_CYAN);
    context.wait_for_all_requests();
    int ret = wait_until(
        stream, context,
        std::chrono::seconds(2),
        {
            overworld,
            dialog,
        }
    );
    context.wait_for(std::chrono::milliseconds(50));

    switch (ret){
    case 0:
        env.log("Error - Stuck in Overworld");
        stats.errors++;
        throw_and_log<FatalProgramException>(
            stream.logger(), ErrorReport::SEND_ERROR_REPORT,
            "Stuck in Overworld.",
            stream
        );

    case 1:
        env.log("Detected Extra Item/Reward");
        return true;

    default:
        return false;
    }
};

void PokemonSV::MassPurchase::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);
    MassPurchase_Descriptor::Stats& stats = env.current_stats<MassPurchase_Descriptor::Stats>();

    send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

    uint16_t item_val = ITEMS;
    bool skip_item = false;
    bool extra = false;
    while (item_val != 0){
        uint16_t qt_val = QUANTITY;
        pbf_press_button(context, BUTTON_A, 20, 105);

        skip_item = mass_purchase(env, env.console, context);

        if (skip_item){
            pbf_press_button(context, BUTTON_B, 20, 105);
        }

        if (!skip_item){
            if (qt_val <= 500){
                uint16_t current = 1;
                while (current + 10 <= qt_val){
                    pbf_press_dpad(context, DPAD_RIGHT, 20, 10);
                    current += 10;
                }
                while (current < qt_val){
                    pbf_press_dpad(context, DPAD_UP, 20, 10);
                    current++;
                }
            }else{
                uint16_t current = 999;
                pbf_press_dpad(context, DPAD_LEFT, 20, 10);
                while (current >= qt_val + 10){
                    pbf_press_dpad(context, DPAD_LEFT, 20, 10);
                    current -= 10;
                }
                while (current > qt_val){
                    pbf_press_dpad(context, DPAD_DOWN, 20, 10);
                    current--;
                }
            }

            pbf_press_button(context, BUTTON_A, 20, 125);

            if (PAY_LP){
                pbf_press_dpad(context, DPAD_DOWN, 5, 105);
            }

            pbf_press_button(context, BUTTON_A, 20, 230);
            pbf_press_button(context, BUTTON_A, 20, 105);
            extra = extra_items(env, env.console, context);
            if (extra){
                pbf_press_button(context, BUTTON_A, 20, 105);
            };

            env.log("Item Purchased");
            stats.total_items++;
            env.update_stats();
            send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
        }
        
        pbf_press_dpad(context, DPAD_DOWN, 20, 105);
        
        item_val--;
    }
        
    pbf_press_button(context, BUTTON_B, 20, 105);
    pbf_press_button(context, BUTTON_B, 20, 105);
    pbf_press_button(context, BUTTON_B, 20, 105);
    pbf_press_button(context, BUTTON_B, 20, 105);
    pbf_press_button(context, BUTTON_B, 20, 105);
    pbf_press_button(context, BUTTON_A, 20, 105);
    pbf_press_button(context, BUTTON_B, 20, 105);
    pbf_press_button(context, BUTTON_A, 20, 105);

    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}






}
}
}
