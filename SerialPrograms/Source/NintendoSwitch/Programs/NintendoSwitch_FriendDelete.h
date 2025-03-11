/*  Friend Delete
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_FriendDelete_H
#define PokemonAutomation_NintendoSwitch_FriendDelete_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class FriendDelete_Descriptor : public SingleSwitchProgramDescriptor{
public:
    FriendDelete_Descriptor();
};



class FriendDelete : public SingleSwitchProgramInstance{
public:
    FriendDelete();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    SimpleIntegerOption<uint16_t> FRIENDS_TO_DELETE;
    BooleanCheckBoxOption BLOCK_FRIENDS;
    MillisecondsOption VIEW_FRIEND_DELAY0;
    MillisecondsOption DELETE_FRIEND_DELAY0;
    MillisecondsOption FINISH_DELETE_DELAY0;
};




}
}
#endif



