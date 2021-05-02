/*  TradeBot
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_TradeBot_H
#define PokemonAutomation_PokemonSwSh_TradeBot_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBox.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "CommonFramework/Options/FixedCode.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class TradeBot : public SingleSwitchProgram{
public:
    TradeBot();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    void trade_slot(const BotBaseContext& context, const uint8_t code[8], uint8_t slot) const;

private:
    FixedCode TRADE_CODE;
    SimpleInteger<uint8_t> BOXES_TO_TRADE;
    BooleanCheckBox LINK_TRADE_EXTRA_LINE;
    TimeExpression<uint16_t> SEARCH_DELAY;
    TimeExpression<uint16_t> CONFIRM_DELAY;
    TimeExpression<uint16_t> TRADE_START;
    TimeExpression<uint16_t> TRADE_COMMUNICATION;
    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> TRADE_ANIMATION;
    TimeExpression<uint16_t> EVOLVE_DELAY;
};



}
}
}
#endif



