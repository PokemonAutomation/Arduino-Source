/*  Surprise Trade
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_SurpriseTrade_H
#define PokemonAutomation_PokemonSwSh_SurpriseTrade_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class SurpriseTrade_Descriptor : public SingleSwitchProgramDescriptor{
public:
    SurpriseTrade_Descriptor();
};



class SurpriseTrade : public SingleSwitchProgramInstance{
public:
    SurpriseTrade();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    void trade_slot(BotBaseContext& context, uint8_t slot, bool next_box) const;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;

    SimpleIntegerOption<uint8_t> BOXES_TO_TRADE;
    TimeExpressionOption<uint16_t> INITIAL_WAIT;
    SectionDividerOption m_advanced_options;
    TimeExpressionOption<uint16_t> TRADE_ANIMATION;
    TimeExpressionOption<uint16_t> EVOLVE_DELAY;
};


}
}
}
#endif



