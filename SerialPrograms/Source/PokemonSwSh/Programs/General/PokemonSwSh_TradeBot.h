/*  TradeBot
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_TradeBot_H
#define PokemonAutomation_PokemonSwSh_TradeBot_H

#include "Common/Qt/Options/StaticTextOption.h"
#include "Common/Qt/Options/BooleanCheckBoxOption.h"
#include "Common/Qt/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/FixedCodeOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
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

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    void trade_slot(BotBaseContext& context, const uint8_t code[8], uint8_t slot) const;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;

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



