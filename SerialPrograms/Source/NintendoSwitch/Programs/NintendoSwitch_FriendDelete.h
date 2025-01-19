/*  Friend Delete
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_FriendDelete_H
#define PokemonAutomation_NintendoSwitch_FriendDelete_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
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
    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;

private:
    SimpleIntegerOption<uint16_t> FRIENDS_TO_DELETE;
    BooleanCheckBoxOption BLOCK_FRIENDS;
    TimeExpressionOption<uint16_t> VIEW_FRIEND_DELAY;
    TimeExpressionOption<uint16_t> DELETE_FRIEND_DELAY;
    TimeExpressionOption<uint16_t> FINISH_DELETE_DELAY;
};




}
}
#endif



