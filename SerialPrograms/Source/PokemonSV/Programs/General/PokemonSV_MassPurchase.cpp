/*  SV Mass Purchase
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_OverworldDetector.h"
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
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

struct MassPurchase_Descriptor::Stats : public StatsTracker {
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
std::unique_ptr<StatsTracker> MassPurchase_Descriptor::make_stats() const {
    return std::unique_ptr<StatsTracker>(new Stats());
}

MassPurchase::MassPurchase()
    : ITEMS(
        "<b>Items to Buy:</b><br>The amount of Items to buy from the postion of the cursor.",
        LockWhileRunning::LOCKED,
        50
    )
    , QUANTITY(
        "<b>Quantity to Buy:</b><br>The amount of each item to buy (Set to 0 for 999).",
        LockWhileRunning::LOCKED,
        0
    )
    , GO_HOME_WHEN_DONE(false)
    , PAY_LP(
        "<b>Pay with LP:</b>",
        LockWhileRunning::UNLOCKED, false
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

bool MassPurchase::mass_purchase(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){
    MassPurchase_Descriptor::Stats& stats = env.current_stats<MassPurchase_Descriptor::Stats>();

    OverworldWatcher overworld(COLOR_RED);
    AdvanceDialogWatcher dialog(COLOR_CYAN);
    context.wait_for_all_requests();
    int ret = wait_until(
        console, context,
        std::chrono::seconds(10),
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
        throw FatalProgramException(env.logger(), "Stuck in Overworld");

    case 1:
        env.log("Detected full bag. Skipped Item");
        stats.skip++;
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
        return true;

    default:
        return false;
    }
};
    
bool MassPurchase::extra_items(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context){
    MassPurchase_Descriptor::Stats& stats = env.current_stats<MassPurchase_Descriptor::Stats>();

    OverworldWatcher overworld(COLOR_RED);
    AdvanceDialogWatcher dialog(COLOR_CYAN);
    context.wait_for_all_requests();
    int ret = wait_until(
        console, context,
        std::chrono::seconds(10),
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
        throw FatalProgramException(env.logger(), "Stuck in Overworld");

    case 1:
        env.log("Detected Extra Item/Reward");
        return true;

    default:
        return false;
    }
};

void PokemonSV::MassPurchase::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    assert_16_9_720p_min(env.logger(), env.console);
    MassPurchase_Descriptor::Stats& stats = env.current_stats<MassPurchase_Descriptor::Stats>();
    
    send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
    
    int item_val = ITEMS;
    bool skip_item = false;
    bool extra = false;
    while (item_val != 0){
        int qt_val = QUANTITY;
        pbf_press_button(context, BUTTON_A, 5, 105);
        
        skip_item = mass_purchase(env, env.console, context);
        
        if (skip_item){
            pbf_press_button(context, BUTTON_B, 5, 105);
        }
        
        if (!skip_item){
            while (qt_val != 0){
                pbf_press_dpad(context, DPAD_UP, 5, 105);
                qt_val--;
            }

            pbf_press_dpad(context, DPAD_DOWN, 5, 105);
            pbf_press_button(context, BUTTON_A, 5, 125);

            if (PAY_LP){
                pbf_press_dpad(context, DPAD_DOWN, 5, 105);
            }

            pbf_press_button(context, BUTTON_A, 5, 250);
            pbf_press_button(context, BUTTON_A, 5, 125);
            extra = extra_items(env, env.console, context);
            if (extra) {
                pbf_press_button(context, BUTTON_A, 5, 125);
            };

            env.log("Item Purchased");
            stats.total_items++;
            env.update_stats();
            send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
        }
        
        pbf_press_dpad(context, DPAD_DOWN, 5, 105);
        
        item_val--;
    }
        
    pbf_press_button(context, BUTTON_B, 5, 105);
    pbf_press_button(context, BUTTON_B, 5, 105);
    pbf_press_button(context, BUTTON_B, 5, 105);
    pbf_press_button(context, BUTTON_B, 5, 105);
    pbf_press_button(context, BUTTON_B, 5, 105);
    pbf_press_button(context, BUTTON_A, 5, 105);
    pbf_press_button(context, BUTTON_B, 5, 105);
    pbf_press_button(context, BUTTON_A, 5, 105);

    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}






}
}
}
