/*  Friend Delete
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_FriendDelete_H
#define PokemonAutomation_NintendoSwitch_FriendDelete_H

#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class FriendDelete_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    FriendDelete_Descriptor();
};



class FriendDelete : public SingleSwitchProgramInstance{
public:
    FriendDelete(const FriendDelete_Descriptor& descriptor);
    virtual void program(SingleSwitchProgramEnvironment& env) override;

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



