/*  TradeBot
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_TradeBot_H
#define PokemonAutomation_PokemonSwSh_TradeBot_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
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

    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;

private:
    void trade_slot(SwitchControllerContext& context, const uint8_t code[8], uint8_t slot) const;

private:
    StartInGripOrGameOption START_LOCATION;

    FixedCodeOption TRADE_CODE;
    SimpleIntegerOption<uint8_t> BOXES_TO_TRADE;
    BooleanCheckBoxOption LINK_TRADE_EXTRA_LINE;
    TimeExpressionOption<uint16_t> SEARCH_DELAY;
    TimeExpressionOption<uint16_t> CONFIRM_DELAY;
    TimeExpressionOption<uint16_t> TRADE_START;
    TimeExpressionOption<uint16_t> TRADE_COMMUNICATION;
    SectionDividerOption m_advanced_options;
    TimeExpressionOption<uint16_t> TRADE_ANIMATION;
    TimeExpressionOption<uint16_t> EVOLVE_DELAY;
};



}
}
}
#endif



