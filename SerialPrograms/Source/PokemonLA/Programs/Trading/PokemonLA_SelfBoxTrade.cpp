/*  Self Box Trade
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonLA_TradeRoutines.h"
#include "PokemonLA_SelfBoxTrade.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;




SelfBoxTrade_Descriptor::SelfBoxTrade_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonLA:SelfBoxTrade",
        STRING_POKEMON + " LA", "Self Box Trade",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/SelfBoxTrade.md",
        "Trade boxes across two Switches.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        2, 2, 2
    )
{}
std::unique_ptr<StatsTracker> SelfBoxTrade_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new TradeStats());
}



SelfBoxTrade::SelfBoxTrade()
    : LANGUAGE_LEFT(
        "<b>Game Language of Left Switch:</b>",
        Pokemon::PokemonNameReader::instance().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , LANGUAGE_RIGHT(
        "<b>Game Language of Right Switch:</b>",
        Pokemon::PokemonNameReader::instance().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , BOXES_TO_TRADE(
        "<b>Number of Boxes to Trade:</b>",
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
    PA_ADD_OPTION(LANGUAGE_LEFT);
    PA_ADD_OPTION(LANGUAGE_RIGHT);
    PA_ADD_OPTION(BOXES_TO_TRADE);
    PA_ADD_OPTION(NOTIFICATIONS);
}





bool SelfBoxTrade::move_to_next(
    MultiSwitchProgramEnvironment& env, CancellableScope& scope,
    uint8_t& row, uint8_t& col
){
    //  Returns true if moved to next box.

    env.log("Moving to next slot.");
    if (col < 5){
        env.run_in_parallel(scope, [](ConsoleHandle& console, ProControllerContext& context){
            pbf_press_dpad(context, DPAD_RIGHT, 20, 140);
        });
        col++;
        return false;
    }
    if (row < 4){
        env.run_in_parallel(scope, [](ConsoleHandle& console, ProControllerContext& context){
            pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
            pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
            pbf_press_dpad(context, DPAD_DOWN, 20, 140);
        });
        col = 0;
        row++;
        return false;
    }
    env.run_in_parallel(scope, [](ConsoleHandle& console, ProControllerContext& context){
        pbf_press_button(context, BUTTON_R, 20, 230);
        pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
        pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
        pbf_press_dpad(context, DPAD_DOWN, 20, 140);
    });
    col = 0;
    row = 0;
    return true;
}


void SelfBoxTrade::program(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
    TradeStats& stats = env.current_stats<TradeStats>();


    //  Connect both controllers.
    env.run_in_parallel(scope, [&](ConsoleHandle& console, ProControllerContext& context){
        pbf_press_button(context, BUTTON_LCLICK, 10, 0);
    });

    uint8_t row = 0;
    uint8_t col = 0;
    for (uint8_t boxes = 0; boxes < BOXES_TO_TRADE;){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        //  Make sure both consoles have selected something.
        std::atomic<bool> ok(true);
        OverlayBoxScope box0(env.consoles[0], {0.925, 0.100, 0.014, 0.030});
        OverlayBoxScope box1(env.consoles[1], {0.925, 0.100, 0.014, 0.030});
        TradeNameReader name_reader0(env.consoles[0], env.consoles[0], LANGUAGE_LEFT);
        TradeNameReader name_reader1(env.consoles[1], env.consoles[1], LANGUAGE_RIGHT);
        env.run_in_parallel(scope, [&](ConsoleHandle& console, ProControllerContext& context){
            auto snapshot = console.video().snapshot();
            ImageStats stats = image_stats(extract_box_reference(snapshot, box0));
            bool is_ok = is_white(stats);
            if (!is_ok){
                console.log("Skipping empty slot.", COLOR_ORANGE);
                ok.store(false, std::memory_order_release);
                return;
            }

            std::string slug = (console.index() == 0 ? name_reader0 : name_reader1).read(snapshot);
            if (slug == "machoke" || slug == "haunter" || slug == "graveler" || slug == "kadabra"){
                console.log("Skipping trade evolution: " + slug, COLOR_RED);
                ok.store(false, std::memory_order_release);
                return;
            }
        });

        if (ok.load(std::memory_order_acquire)){
            //  Perform trade.
            MultiConsoleErrorState error_state;
            env.run_in_parallel(scope, [&](ConsoleHandle& console, ProControllerContext& context){
                trade_current_pokemon(console, context, error_state, stats);
            });
            stats.m_trades++;
        }else{
            stats.m_errors++;
        }

        //  Move to next slot.
        if (move_to_next(env, scope, row, col)){
            boxes++;
        }
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}







}
}
}
