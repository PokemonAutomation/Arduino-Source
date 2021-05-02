/*  Surprise Trade
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_SurpriseTrade_H
#define PokemonAutomation_PokemonSwSh_SurpriseTrade_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class SurpriseTrade : public SingleSwitchProgram{
public:
    SurpriseTrade();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    void trade_slot(const BotBaseContext& context, uint8_t slot, bool next_box) const;

private:
    SimpleInteger<uint8_t> BOXES_TO_TRADE;
    TimeExpression<uint16_t> INITIAL_WAIT;
    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> TRADE_ANIMATION;
    TimeExpression<uint16_t> EVOLVE_DELAY;
};


}
}
}
#endif



