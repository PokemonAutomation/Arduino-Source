/*  Self Touch Trade
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Options/Pokemon_NameSelectWidget.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonLA/Resources/PokemonLA_AvailablePokemon.h"
#include "PokemonLA_TradeRoutines.h"
#include "PokemonLA_SelfTouchTrade.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



SelfTouchTrade_Descriptor::SelfTouchTrade_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonLA:SelfTouchTrade",
        STRING_POKEMON + " LA", "Self Touch Trade",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/SelfTouchTrade.md",
        "Repeatedly trade " + STRING_POKEMON + " between two local Switches to fill up research.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB,
        2, 2, 2
    )
{}



SelfTouchTrade::SelfTouchTrade(const SelfTouchTrade_Descriptor& descriptor)
    : MultiSwitchProgramInstance(descriptor)
    , LANGUAGE("<b>Game Language of the Hosting Switch:</b>", Pokemon::PokemonNameReader::instance().languages(), true)
    , HOSTING_SWITCH(
        "<b>Host Switch:</b><br>This is the Switch hosting the " + STRING_POKEMON + " to be touch-traded to the other.",
        {
            "Switch 0 (Left)",
            "Switch 1 (Right)",
        },
        0
    )
    , BOXES_TO_TRADE(
        "<b>Number of Boxes to Touch-Trade:</b>",
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



std::unique_ptr<StatsTracker> SelfTouchTrade::make_stats() const{
    return std::unique_ptr<StatsTracker>(new TradeStats());
}




bool SelfTouchTrade::trade_one(MultiSwitchProgramEnvironment& env, std::map<std::string, int>& trades_left){
    TradeStats& stats = env.stats<TradeStats>();

    ConsoleHandle& host = HOSTING_SWITCH == 0 ? env.consoles[0] : env.consoles[1];
//    ConsoleHandle& recv = HOSTING_SWITCH == 0 ? env.consoles[1] : env.consoles[0];

    //  Read the name and see if the receiver still needs it.
    TradeNameReader name_reader(host, host, LANGUAGE);
    QImage image = host.video().snapshot();
    std::string slug = name_reader.read(image);
    auto iter = trades_left.find(slug);
    if (iter == trades_left.end()){
        host.log("Unable to read name. Moving on...", COLOR_RED);
        stats.m_errors++;
        dump_image(host, env.program_info(), "ReadName", image);
        return false;
    }
    if (iter->second <= 0){
        host.log(STRING_POKEMON + " not needed anymore. Moving on...");
        return false;
    }
    if (slug == "machoke" || slug == "haunter" || slug == "graveler" || slug == "kadabra"){
        host.log("Skipping trade evolution: " + slug, COLOR_RED);
        return false;
    }

    //  Perform trade.
    host.log("\"" + slug + "\" - Trades Remaining: " + std::to_string(iter->second));
#if 1
    MultiConsoleErrorState error_state;
    env.run_in_parallel([&](ConsoleHandle& console){
        trade_current_pokemon(env, console, error_state, stats);
    });
    stats.m_trades++;
    iter->second--;
#else
    env.wait_for(std::chrono::milliseconds(5000));
    return false;
#endif
    env.wait_for(std::chrono::milliseconds(500));

    return true;
}
bool SelfTouchTrade::move_to_next(ConsoleHandle& host, uint8_t& row, uint8_t& col){
    //  Returns true if moved to next box.

    host.log("Moving to next slot.");
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
    TradeStats& stats = env.stats<TradeStats>();

    //  Build list of what's needed.
    std::map<std::string, int> trades_left;
    for (const std::pair<std::string, int>& item : TRADE_COUNTS.list()){
        trades_left[item.first] = item.second;
    }

    //  Connect both controllers.
    env.run_in_parallel([&](ConsoleHandle& console){
        pbf_press_button(console, BUTTON_LCLICK, 10, 0);
    });

    uint8_t row = 0;
    uint8_t col = 0;

    ConsoleHandle& host = HOSTING_SWITCH == 0 ? env.consoles[0] : env.consoles[1];
    ConsoleHandle& recv = HOSTING_SWITCH == 0 ? env.consoles[1] : env.consoles[0];

    for (uint8_t boxes = 0; boxes < BOXES_TO_TRADE;){
        env.update_stats();
        send_program_status_notification(
            env.logger(), NOTIFICATION_STATUS_UPDATE,
            env.program_info(),
            "",
            stats.to_str()
        );

        //  Make sure both consoles have selected something.
        bool host_ok, recv_ok;
        InferenceBoxScope box0(host, {0.925, 0.100, 0.014, 0.030});
        InferenceBoxScope box1(recv, {0.925, 0.100, 0.014, 0.030});
        env.run_in_parallel([&](ConsoleHandle& console){
            ImageStats stats = image_stats(extract_box_reference(console.video().snapshot(), box0));
            bool ok = is_white(stats);
            if (host.index() == console.index()){
                host_ok = ok;
            }else{
                recv_ok = ok;
            }
        });

        if (!recv_ok){
            throw UserSetupError(recv, "Receiving Switch has not selected a " + STRING_POKEMON.toStdString() + ".");
        }

        //  Perform trade.
        bool traded = false;
        if (host_ok){
            traded = trade_one(env, trades_left);
        }else{
            recv.log("Skipping empty slot on host...", COLOR_PURPLE);
        }

        //  Move to next slot.
        if (!traded){
            if (move_to_next(host, row, col)){
                boxes++;
            }
            host.botbase().wait_for_all_requests();
        }
    }

    env.update_stats();
    send_program_finished_notification(
        env.logger(), NOTIFICATION_PROGRAM_FINISH,
        env.program_info(),
        "",
        stats.to_str()
    );
}



}
}
}
