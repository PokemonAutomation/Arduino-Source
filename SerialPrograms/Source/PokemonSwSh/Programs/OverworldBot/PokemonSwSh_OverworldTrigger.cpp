/*  Overworld Trigger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Compiler.h"
#include "Common/Cpp/Exceptions.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh_OverworldMovement.h"
#include "PokemonSwSh_OverworldTrigger.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


OverworldTrigger::OverworldTrigger(OverworldTargetTracker& target_tracker)
    : m_target_tracker(target_tracker)
{}
void OverworldTrigger::whistle(ProControllerContext& context, bool rotate){
    context.wait_for_all_requests();
    m_target_tracker.set_stop_on_target(false);

    if (rotate){
        pbf_move_right_joystick(context, 192, 255, 50, 70);
    }
    pbf_press_button(context, BUTTON_LCLICK, 5, 0);
    pbf_mash_button(context, BUTTON_B, 120);

    context.wait_for_all_requests();
    m_target_tracker.set_stop_on_target(true);
    if (m_target_tracker.has_good_target()){
        throw OperationCancelledException();
    }

    pbf_mash_button(context, BUTTON_B, 40);

    context.wait_for_all_requests();
    m_target_tracker.set_stop_on_target(false);
}


void OverworldTrigger_Whistle::run(ProControllerContext& context){
    whistle(context, !m_first_after_battle);
    m_first_after_battle = false;
}


OverworldTrigger_WhistleStaticAction::OverworldTrigger_WhistleStaticAction(
    OverworldTargetTracker& target_tracker,
    bool whistle_first,
    size_t whistle_count,
    size_t action_count
)
    : OverworldTrigger(target_tracker)
    , m_whistle_first(whistle_first)
    , m_whistle_count(whistle_count)
    , m_action_count(action_count)
{}
void OverworldTrigger_WhistleStaticAction::run(ProControllerContext& context){
    if (m_whistle_first){
        whistle_loop(context);
        action_loop(context);
    }else{
        action_loop(context);
        whistle_loop(context);
    }
}
void OverworldTrigger_WhistleStaticAction::whistle_loop(ProControllerContext& context){
    for (size_t c = 0; c < m_whistle_count; c++){
        whistle(context, !m_first_after_battle);
        m_first_after_battle = false;
    }
}
void OverworldTrigger_WhistleStaticAction::action_loop(ProControllerContext& context){
    m_target_tracker.set_stop_on_target(true);
    context.wait_for_all_requests();
    for (size_t c = 0; c < m_action_count; c++){
        action(context);
        m_first_after_battle = false;
    }
    context.wait_for_all_requests();
    m_target_tracker.set_stop_on_target(false);
}


void OverworldTrigger_WhistleCircle::action(ProControllerContext& context){
    circle_in_place(context, rand() % 2);
}
void OverworldTrigger_WhistleHorizontal::action(ProControllerContext& context){
    move_in_line(context, true);
}
void OverworldTrigger_WhistleVertical::action(ProControllerContext& context){
    move_in_line(context, false);
}



}
}
}



