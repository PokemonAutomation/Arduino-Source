/*  Friend Code Adder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_FriendCodeAdder_H
#define PokemonAutomation_NintendoSwitch_FriendCodeAdder_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_FriendCodeListOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class FriendCodeAdder_Descriptor : public SingleSwitchProgramDescriptor{
public:
    FriendCodeAdder_Descriptor();
};


class FriendCodeAdder : public SingleSwitchProgramInstance{
public:
    FriendCodeAdder();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    SimpleIntegerOption<uint8_t> USER_SLOT;
    FriendCodeListOption FRIEND_CODES;
    SectionDividerOption m_advanced_options;
    MillisecondsOption OPEN_CODE_PAD_DELAY1;
    MillisecondsOption SEARCH_TIME0;
    MillisecondsOption TOGGLE_BEST_STATUS_DELAY0;
};


}
}
#endif

