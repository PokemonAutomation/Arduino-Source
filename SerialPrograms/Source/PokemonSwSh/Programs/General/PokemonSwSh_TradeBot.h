/*  TradeBot
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_TradeBot_H
#define PokemonAutomation_PokemonSwSh_TradeBot_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "Common/Cpp/Options/FixedCodeOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class TradeBot_Descriptor : public SingleSwitchProgramDescriptor{
public:
    TradeBot_Descriptor();
};



class TradeBot : public SingleSwitchProgramInstance{
public:
    TradeBot();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    void trade_slot(
        ConsoleHandle& console, ProControllerContext& context,
        const std::string& code, uint8_t slot
    ) const;

private:
    StartInGripOrGameOption START_LOCATION;

    FixedCodeOption TRADE_CODE;
    SimpleIntegerOption<uint8_t> BOXES_TO_TRADE;
    BooleanCheckBoxOption LINK_TRADE_EXTRA_LINE;
    MillisecondsOption SEARCH_DELAY0;
    MillisecondsOption CONFIRM_DELAY0;
    MillisecondsOption TRADE_START0;
    MillisecondsOption TRADE_COMMUNICATION0;
    SectionDividerOption m_advanced_options;
    MillisecondsOption TRADE_ANIMATION0;
    MillisecondsOption EVOLVE_DELAY0;
};



}
}
}
#endif



