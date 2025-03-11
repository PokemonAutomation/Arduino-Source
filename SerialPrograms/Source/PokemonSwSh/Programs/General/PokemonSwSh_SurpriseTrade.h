/*  Surprise Trade
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_SurpriseTrade_H
#define PokemonAutomation_PokemonSwSh_SurpriseTrade_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
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

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    void trade_slot(ProControllerContext& context, uint8_t slot, bool next_box) const;

private:
    StartInGripOrGameOption START_LOCATION;

    SimpleIntegerOption<uint8_t> BOXES_TO_TRADE;
    MillisecondsOption INITIAL_WAIT0;
    SectionDividerOption m_advanced_options;
    MillisecondsOption TRADE_ANIMATION0;
    MillisecondsOption EVOLVE_DELAY0;
};


}
}
}
#endif



