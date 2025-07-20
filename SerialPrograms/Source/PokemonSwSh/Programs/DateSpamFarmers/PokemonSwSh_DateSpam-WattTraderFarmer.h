/*  Watt Trader Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_WattTraderFarmer_H
#define PokemonAutomation_PokemonSwSh_WattTraderFarmer_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class WattTraderFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    WattTraderFarmer_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class WattTraderFarmer : public SingleSwitchProgramInstance{
public:
    WattTraderFarmer();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    void buy_one(SingleSwitchProgramEnvironment& env, ProControllerContext& context);


};



}
}
}
#endif



