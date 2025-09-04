/*  Friend Delete
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

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
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}


FriendDelete::FriendDelete()
    : FRIENDS_TO_DELETE(
        "<b>Number of Friends to Delete:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        3, 0, 300
    )
    , BLOCK_FRIENDS(
        "<b>Block Friends:</b><br>Block instead of delete!",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , VIEW_FRIEND_DELAY0(
        "<b>View Friend Delay:</b><br>Delay from opening a friend to when you can press buttons.",
        LockMode::LOCK_WHILE_RUNNING,
        "2000 ms"
    )
    , DELETE_FRIEND_DELAY0(
        "<b>Delete Friend Delay:</b><br>Delay to delete the friend.",
        LockMode::LOCK_WHILE_RUNNING,
        "8000 ms"
    )
    , FINISH_DELETE_DELAY0(
        "<b>Finish Delete Delay:</b><br>Delay after deleting a friend.",
        LockMode::LOCK_WHILE_RUNNING,
        "2000 ms"
    )
{
    PA_ADD_OPTION(FRIENDS_TO_DELETE);
    PA_ADD_OPTION(BLOCK_FRIENDS);
    PA_ADD_OPTION(VIEW_FRIEND_DELAY0);
    PA_ADD_OPTION(DELETE_FRIEND_DELAY0);
    PA_ADD_OPTION(FINISH_DELETE_DELAY0);
}
void FriendDelete::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    pbf_press_button(context, BUTTON_A, 10, 20);

    for (uint16_t c = 0; c < FRIENDS_TO_DELETE; c++){
        pbf_press_button(context, BUTTON_A, 40ms, VIEW_FRIEND_DELAY0);      //  View friend
        pbf_press_dpad(context, DPAD_DOWN, 10, 20);
        pbf_press_button(context, BUTTON_A, 10, 90);                        //  Click on Options
        if (BLOCK_FRIENDS){
            pbf_press_dpad(context, DPAD_DOWN, 10, 20);
        }
        pbf_press_button(context, BUTTON_A, 10, 90);                        //  Click on Remove/Block Friend
        if (BLOCK_FRIENDS){
            pbf_press_button(context, BUTTON_A, 80ms, VIEW_FRIEND_DELAY0);  //  Confirm
        }
        pbf_press_button(context, BUTTON_A, 80ms, DELETE_FRIEND_DELAY0);    //  Confirm
        pbf_press_button(context, BUTTON_A, 80ms, FINISH_DELETE_DELAY0);    //  Finish delete friend.
    }
}






}
}
