/*  Friend Delete
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_FriendDelete_H
#define PokemonAutomation_NintendoSwitch_FriendDelete_H

#include "CommonFramework/Options/BooleanCheckBox.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class FriendDelete : public SingleSwitchProgram{
public:
    FriendDelete();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    SimpleInteger<uint16_t> FRIENDS_TO_DELETE;
    BooleanCheckBox BLOCK_FRIENDS;
    TimeExpression<uint16_t> VIEW_FRIEND_DELAY;
    TimeExpression<uint16_t> DELETE_FRIEND_DELAY;
    TimeExpression<uint16_t> FINISH_DELETE_DELAY;
};



}
}
#endif



