/*  Friend Code Adder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_FriendCodeAdder_H
#define PokemonAutomation_NintendoSwitch_FriendCodeAdder_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Options/FriendCodeList.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class FriendCodeAdder_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    FriendCodeAdder_Descriptor();
};


class FriendCodeAdder : public SingleSwitchProgramInstance{
public:
    FriendCodeAdder(const FriendCodeAdder_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    SimpleInteger<uint8_t> USER_SLOT;
    FriendCodeList FRIEND_CODES;
    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> OPEN_CODE_PAD_DELAY;
    TimeExpression<uint16_t> SEARCH_TIME;
    TimeExpression<uint16_t> TOGGLE_BEST_STATUS_DELAY;
};


}
}
#endif

