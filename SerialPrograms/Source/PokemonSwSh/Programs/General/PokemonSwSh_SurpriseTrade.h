/*  Surprise Trade
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_SurpriseTrade_H
#define PokemonAutomation_PokemonSwSh_SurpriseTrade_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
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

    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;

private:
    void trade_slot(SwitchControllerContext& context, uint8_t slot, bool next_box) const;

private:
    StartInGripOrGameOption START_LOCATION;

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



