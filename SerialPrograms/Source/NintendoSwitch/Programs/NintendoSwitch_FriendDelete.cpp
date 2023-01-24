/*  Friend Delete
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch_FriendDelete.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


FriendDelete_Descriptor::FriendDelete_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:FriendDelete",
        "Nintendo Switch", "Friend Delete",
        "ComputerControl/blob/master/Wiki/Programs/NintendoSwitch/FriendDelete.md",
        "Mass delete/block all those unwanted friends.",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


FriendDelete::FriendDelete()
    : FRIENDS_TO_DELETE(
        "<b>Number of Friends to Delete:</b>",
        LockWhileRunning::LOCKED,
        3, 0, 300
    )
    , BLOCK_FRIENDS(
        "<b>Block Friends:</b><br>Block instead of delete!",
        LockWhileRunning::LOCKED,
        false
    )
    , VIEW_FRIEND_DELAY(
        "<b>View Friend Delay:</b><br>Delay from opening a friend to when you can press buttons.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "2 * TICKS_PER_SECOND"
    )
    , DELETE_FRIEND_DELAY(
        "<b>Delete Friend Delay:</b><br>Delay to delete the friend.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "8 * TICKS_PER_SECOND"
    )
    , FINISH_DELETE_DELAY(
        "<b>Finish Delete Delay:</b><br>Delay after deleting a friend.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "2 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(FRIENDS_TO_DELETE);
    PA_ADD_OPTION(BLOCK_FRIENDS);
    PA_ADD_OPTION(VIEW_FRIEND_DELAY);
    PA_ADD_OPTION(DELETE_FRIEND_DELAY);
    PA_ADD_OPTION(FINISH_DELETE_DELAY);
}
void FriendDelete::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    pbf_press_button(context, BUTTON_A, 5, 5);

    for (uint16_t c = 0; c < FRIENDS_TO_DELETE; c++){
        pbf_press_button(context, BUTTON_A, 5, VIEW_FRIEND_DELAY);      //  View friend
        pbf_press_dpad(context, DPAD_DOWN, 5, 5);
        pbf_press_button(context, BUTTON_A, 10, 90);                    //  Click on Options
        if (BLOCK_FRIENDS){
            pbf_press_dpad(context, DPAD_DOWN, 5, 5);
        }
        pbf_press_button(context, BUTTON_A, 10, 90);                    //  Click on Remove/Block Friend
        if (BLOCK_FRIENDS){
            pbf_press_button(context, BUTTON_A, 5, VIEW_FRIEND_DELAY);  //  Confirm
        }
        pbf_press_button(context, BUTTON_A, 5, DELETE_FRIEND_DELAY);    //  Confirm
        pbf_press_button(context, BUTTON_A, 5, FINISH_DELETE_DELAY);    //  Finish delete friend.
    }
}






}
}
