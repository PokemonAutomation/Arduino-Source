/*  Overworld Trigger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "PokemonSwSh_OverworldMovement.h"
#include "PokemonSwSh_OverworldTrigger.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


OverworldTrigger::OverworldTrigger(ProgramEnvironment& env)
    : m_env(env)
{}
void OverworldTrigger::whistle(const BotBaseContext& context, bool rotate){
    if (rotate){
        pbf_move_right_joystick(context, 192, 255, 50, 70);
    }
    pbf_press_button(context, BUTTON_LCLICK, 5, 0);
    pbf_mash_button(context, BUTTON_B, 120);
}


void OverworldTrigger_Whistle::run(
    InterruptableCommandSession& session,
    OverworldTargetTracker& target_tracker
){
//    m_env.log("Whistle and wait.");
    session.run([=](const BotBaseContext& context){
        whistle(context, !m_first_after_battle);
        context.botbase().wait_for_all_requests();
    });
    m_first_after_battle = false;
}


OverworldTrigger_WhistleStaticAction::OverworldTrigger_WhistleStaticAction(
    ProgramEnvironment& env,
    bool whistle_first,
    size_t whistle_count,
    size_t action_count
)
    : OverworldTrigger(env)
    , m_whistle_first(whistle_first)
    , m_whistle_count(whistle_count)
    , m_action_count(action_count)
{}
void OverworldTrigger_WhistleStaticAction::run(
    InterruptableCommandSession& session,
    OverworldTargetTracker& target_tracker
){
    target_tracker.set_stop_on_target(true);
    if (m_whistle_first){
        session.run([=](const BotBaseContext& context){
            for (size_t c = 0; c < m_whistle_count; c++){
                whistle(context, !m_first_after_battle);
                m_first_after_battle = false;
            }
            for (size_t c = 0; c < m_action_count; c++){
                action(context);
            }
            context.botbase().wait_for_all_requests();
        });
    }else{
        session.run([=](const BotBaseContext& context){
            for (size_t c = 0; c < m_action_count; c++){
                action(context);
            }
            for (size_t c = 0; c < m_whistle_count; c++){
                whistle(context, true);
            }
            context.botbase().wait_for_all_requests();
        });

    }
    target_tracker.set_stop_on_target(false);
}


void OverworldTrigger_WhistleCircle::action(const BotBaseContext& context){
    circle_in_place(context, rand() % 2);
}
void OverworldTrigger_WhistleHorizontal::action(const BotBaseContext& context){
    move_in_line(context, true);
}
void OverworldTrigger_WhistleVertical::action(const BotBaseContext& context){
    move_in_line(context, false);
}



}
}
}



