/*  Friend Delete
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "FriendDelete.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


FriendDelete_Descriptor::FriendDelete_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "NintendoSwitch:FriendDelete",
        "Friend Delete",
        "NativePrograms/FriendDelete.md",
        "Mass delete/block all those unwanted friends.",
        FeedbackType::NONE,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


FriendDelete::FriendDelete(const FriendDelete_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , FRIENDS_TO_DELETE(
        "<b>Number of Friends to Delete:</b>",
        3, 0, 300
    )
    , BLOCK_FRIENDS(
        "<b>Block Friends:</b><br>Block instead of delete!",
        false
    )
    , VIEW_FRIEND_DELAY(
        "<b>View Friend Delay:</b><br>Delay from opening a friend to when you can press buttons.",
        "2 * TICKS_PER_SECOND"
    )
    , DELETE_FRIEND_DELAY(
        "<b>Delete Friend Delay:</b><br>Delay to delete the friend.",
        "8 * TICKS_PER_SECOND"
    )
    , FINISH_DELETE_DELAY(
        "<b>Finish Delete Delay:</b><br>Delay after deleting a friend.",
        "2 * TICKS_PER_SECOND"
    )
{
    m_options.emplace_back(&FRIENDS_TO_DELETE, "FRIENDS_TO_DELETE");
    m_options.emplace_back(&BLOCK_FRIENDS, "BLOCK_FRIENDS");
    m_options.emplace_back(&VIEW_FRIEND_DELAY, "VIEW_FRIEND_DELAY");
    m_options.emplace_back(&DELETE_FRIEND_DELAY, "DELETE_FRIEND_DELAY");
    m_options.emplace_back(&FINISH_DELETE_DELAY, "FINISH_DELETE_DELAY");
}
void FriendDelete::program(SingleSwitchProgramEnvironment& env){
    pbf_press_button(env.console, BUTTON_A, 5, 5);

    for (uint16_t c = 0; c < FRIENDS_TO_DELETE; c++){
        pbf_press_button(env.console, BUTTON_A, 5, VIEW_FRIEND_DELAY);      //  View friend
        pbf_press_dpad(env.console, DPAD_DOWN, 5, 5);
        pbf_press_button(env.console, BUTTON_A, 10, 90);                    //  Click on Options
        if (BLOCK_FRIENDS){
            pbf_press_dpad(env.console, DPAD_DOWN, 5, 5);
        }
        pbf_press_button(env.console, BUTTON_A, 10, 90);                    //  Click on Remove/Block Friend
        if (BLOCK_FRIENDS){
            pbf_press_button(env.console, BUTTON_A, 5, VIEW_FRIEND_DELAY);  //  Confirm
        }
        pbf_press_button(env.console, BUTTON_A, 5, DELETE_FRIEND_DELAY);    //  Confirm
        pbf_press_button(env.console, BUTTON_A, 5, FINISH_DELETE_DELAY);    //  Finish delete friend.
    }

    end_program_callback(env.console);
    end_program_loop(env.console);
}






}
}
