/*  Self Touch Trade
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonLA_TradeRoutines.h"
#include "PokemonLA_SelfTouchTrade.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;



SelfTouchTrade_Descriptor::SelfTouchTrade_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonLA:SelfTouchTrade",
        STRING_POKEMON + " LA", "Self Touch Trade",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/SelfTouchTrade.md",
        "Repeatedly trade " + STRING_POKEMON + " between two local Switches to fill up research.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        2, 2, 2
    )
{}
std::unique_ptr<StatsTracker> SelfTouchTrade_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new TradeStats());
}



SelfTouchTrade::SelfTouchTrade()
    : LANGUAGE(
        "<b>Game Language of the Hosting Switch:</b>",
        Pokemon::PokemonNameReader::instance().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , HOSTING_SWITCH(
        "<b>Host Switch:</b><br>This is the Switch hosting the " + STRING_POKEMON + " to be touch-traded to the other.",
        {
            {HostingSwitch::Switch0, "switch0", "Switch 0 (Left)"},
            {HostingSwitch::Switch1, "switch1", "Switch 1 (Right)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        HostingSwitch::Switch0
    )
    , BOXES_TO_TRADE(
        "<b>Number of Boxes to Touch-Trade:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        2, 0, 32
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(HOSTING_SWITCH);
    PA_ADD_OPTION(BOXES_TO_TRADE);
    PA_ADD_OPTION(TRADE_COUNTS);
    PA_ADD_OPTION(NOTIFICATIONS);
}




bool SelfTouchTrade::trade_one(
    MultiSwitchProgramEnvironment& env, CancellableScope& scope,
    std::map<std::string, SimpleIntegerCell<uint8_t>*>& trades_left
){
    TradeStats& stats = env.current_stats<TradeStats>();

    ConsoleHandle& host = HOSTING_SWITCH == HostingSwitch::Switch0 ? env.consoles[0] : env.consoles[1];

    //  Read the name and see if the receiver still needs it.
    TradeNameReader name_reader(host, host, LANGUAGE);
    VideoSnapshot snapshot = host.video().snapshot();
    std::string slug = name_reader.read(snapshot);
    auto iter = trades_left.find(slug);
    if (iter == trades_left.end()){
        host.log("Unable to read name. Moving on...", COLOR_RED);
        stats.m_errors++;
        dump_image(host, env.program_info(), "ReadName", snapshot);
        return false;
    }
    uint8_t current_trades_left = iter->second->current_value();
    if (current_trades_left <= 0){
        host.log(STRING_POKEMON + " not needed anymore. Moving on...");
        return false;
    }
    if (slug == "machoke" || slug == "haunter" || slug == "graveler" || slug == "kadabra"){
        host.log("Skipping trade evolution: " + slug, COLOR_RED);
        return false;
    }

    //  Perform trade.
    host.log("\"" + slug + "\" - Trades Remaining: " + std::to_string(current_trades_left));
#if 1
    MultiConsoleErrorState error_state;
    env.run_in_parallel(scope, [&](ConsoleHandle& console, ProControllerContext& context){
        trade_current_pokemon(console, context, error_state, stats);
    });
    stats.m_trades++;
    iter->second->set(current_trades_left - 1);
#else
    env.wait_for(std::chrono::milliseconds(5000));
    return false;
#endif
    scope.wait_for(std::chrono::milliseconds(500));

    return true;
}
bool SelfTouchTrade::move_to_next(Logger& logger, ProControllerContext& host, uint8_t& row, uint8_t& col){
    //  Returns true if moved to next box.

    logger.log("Moving to next slot.");
    if (col < 5){
        pbf_press_dpad(host, DPAD_RIGHT, 20, 140);
        col++;
        return false;
    }
    if (row < 4){
        pbf_press_dpad(host, DPAD_RIGHT, 20, 105);
        pbf_press_dpad(host, DPAD_RIGHT, 20, 105);
        pbf_press_dpad(host, DPAD_DOWN, 20, 140);
        col = 0;
        row++;
        return false;
    }
    pbf_press_button(host, BUTTON_R, 20, 230);
    pbf_press_dpad(host, DPAD_RIGHT, 20, 105);
    pbf_press_dpad(host, DPAD_RIGHT, 20, 105);
    pbf_press_dpad(host, DPAD_DOWN, 20, 140);
    col = 0;
    row = 0;
    return true;
}


void SelfTouchTrade::program(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
    //  Build list of what's needed.
    std::map<std::string, SimpleIntegerCell<uint8_t>*> trades_left;
    for (StaticTableRow* item : TRADE_COUNTS.table()){
        trades_left[item->slug()] = &static_cast<TradeCountTableRow&>(*item).count;
    }

    //  Connect both controllers.
    env.run_in_parallel(scope, [&](ConsoleHandle& console, ProControllerContext& context){
        pbf_press_button(context, BUTTON_LCLICK, 10, 0);
    });

    uint8_t row = 0;
    uint8_t col = 0;

    bool host0 = HOSTING_SWITCH == HostingSwitch::Switch0;
    ProControllerContext host_context(scope, (host0 ? env.consoles[0] : env.consoles[1]).controller<ProController>());
    ConsoleHandle& host = host0 ? env.consoles[0] : env.consoles[1];
    ConsoleHandle& recv = host0 ? env.consoles[1] : env.consoles[0];

    for (uint8_t boxes = 0; boxes < BOXES_TO_TRADE;){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        //  Make sure both consoles have selected something.
        bool host_ok, recv_ok;
        OverlayBoxScope box0(host, {0.925, 0.100, 0.014, 0.030});
        OverlayBoxScope box1(recv, {0.925, 0.100, 0.014, 0.030});
        env.run_in_parallel(scope, [&](ConsoleHandle& console, ProControllerContext& context){
            ImageStats stats = image_stats(extract_box_reference(console.video().snapshot(), box0));
            bool ok = is_white(stats);
            if (host.index() == console.index()){
                host_ok = ok;
            }else{
                recv_ok = ok;
            }
        });

        if (!recv_ok){
            throw UserSetupError(recv, "Receiving Switch has not selected a " + STRING_POKEMON + ".");
        }

        //  Perform trade.
        bool traded = false;
        if (host_ok){
            traded = trade_one(env, scope, trades_left);
        }else{
            recv.log("Skipping empty slot on host...", COLOR_PURPLE);
        }

        //  Move to next slot.
        if (!traded){
            if (move_to_next(host, host_context, row, col)){
                boxes++;
            }
            host_context.wait_for_all_requests();
        }
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}



}
}
}
