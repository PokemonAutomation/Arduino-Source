/*  Friend Code Adder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_FriendCodeAdder_H
#define PokemonAutomation_NintendoSwitch_FriendCodeAdder_H

#include "CommonFramework/Options/StaticTextOption.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_FriendCodeListOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"

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
    SimpleIntegerOption<uint8_t> USER_SLOT;
    FriendCodeListOption FRIEND_CODES;
    SectionDividerOption m_advanced_options;
    TimeExpressionOption<uint16_t> OPEN_CODE_PAD_DELAY;
    TimeExpressionOption<uint16_t> SEARCH_TIME;
    TimeExpressionOption<uint16_t> TOGGLE_BEST_STATUS_DELAY;
};


}
}
#endif

