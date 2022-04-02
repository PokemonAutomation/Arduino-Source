/*  Self Box Trade
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonLA_TradeRoutines.h"
#include "PokemonLA_SelfBoxTrade.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{




SelfBoxTrade_Descriptor::SelfBoxTrade_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonLA:SelfBoxTrade",
        STRING_POKEMON + " LA", "Self Box Trade",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/SelfBoxTrade.md",
        "Trade boxes across two Switches.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB,
        2, 2, 2
    )
{}



SelfBoxTrade::SelfBoxTrade(const SelfBoxTrade_Descriptor& descriptor)
    : MultiSwitchProgramInstance(descriptor)
    , LANGUAGE_LEFT("<b>Game Language of Left Switch:</b>", Pokemon::PokemonNameReader::instance().languages(), true)
    , LANGUAGE_RIGHT("<b>Game Language of Right Switch:</b>", Pokemon::PokemonNameReader::instance().languages(), true)
    , BOXES_TO_TRADE(
        "<b>Number of Boxes to Trade:</b>",
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



std::unique_ptr<StatsTracker> SelfBoxTrade::make_stats() const{
    return std::unique_ptr<StatsTracker>(new TradeStats());
}


bool SelfBoxTrade::move_to_next(MultiSwitchProgramEnvironment& env, uint8_t& row, uint8_t& col){
    //  Returns true if moved to next box.

    env.log("Moving to next slot.");
    if (col < 5){
        env.run_in_parallel([](BotBaseContext& context, const ConsoleHandle& console){
            pbf_press_dpad(context, DPAD_RIGHT, 20, 140);
        });
        col++;
        return false;
    }
    if (row < 4){
        env.run_in_parallel([](BotBaseContext& context, const ConsoleHandle& console){
            pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
            pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
            pbf_press_dpad(context, DPAD_DOWN, 20, 140);
        });
        col = 0;
        row++;
        return false;
    }
    env.run_in_parallel([](BotBaseContext& context, const ConsoleHandle& console){
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
    TradeStats& stats = env.stats<TradeStats>();


    //  Connect both controllers.
    env.run_in_parallel([&](BotBaseContext& context, ConsoleHandle& console){
        pbf_press_button(context, BUTTON_LCLICK, 10, 0);
    });

    uint8_t row = 0;
    uint8_t col = 0;
    for (uint8_t boxes = 0; boxes < BOXES_TO_TRADE;){
        env.update_stats();
        send_program_status_notification(
            env.logger(), NOTIFICATION_STATUS_UPDATE,
            env.program_info(),
            "",
            stats.to_str()
        );

        //  Make sure both consoles have selected something.
        std::atomic<bool> ok(true);
        InferenceBoxScope box0(env.consoles[0], {0.920, 0.100, 0.020, 0.030});
        InferenceBoxScope box1(env.consoles[1], {0.920, 0.100, 0.020, 0.030});
        TradeNameReader name_reader0(env.consoles[0], env.consoles[0], LANGUAGE_LEFT);
        TradeNameReader name_reader1(env.consoles[1], env.consoles[1], LANGUAGE_RIGHT);
        env.run_in_parallel([&](BotBaseContext& context, ConsoleHandle& console){
            ConstImageRef image0 = extract_box_reference(console.video().snapshot(), box0);
            ImageStats stats = image_stats(image0);
            bool is_ok = is_white(stats);
            if (!is_ok){
                console.log("Skipping empty slot.", COLOR_ORANGE);
                ok.store(false, std::memory_order_release);
                return;
            }

            QImage image1 = console.video().snapshot();
            std::string slug = (console.index() == 0 ? name_reader0 : name_reader1).read(image1);
            if (slug == "machoke" || slug == "haunter" || slug == "graveler" || slug == "kadabra"){
                console.log("Skipping trade evolution: " + slug, COLOR_RED);
                ok.store(false, std::memory_order_release);
                return;
            }
        });

        if (ok.load(std::memory_order_acquire)){
            //  Perform trade.
            MultiConsoleErrorState error_state;
            env.run_in_parallel([&](BotBaseContext& context, ConsoleHandle& console){
                trade_current_pokemon(env, context, console, error_state, stats);
            });
            stats.m_trades++;
        }else{
            stats.m_errors++;
        }

        //  Move to next slot.
        if (move_to_next(env, row, col)){
            boxes++;
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
