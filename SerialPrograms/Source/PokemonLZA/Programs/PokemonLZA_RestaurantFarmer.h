/*  Restaurant Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_RestaurantFarmer_H
#define PokemonAutomation_PokemonLZA_RestaurantFarmer_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


class RestaurantFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    RestaurantFarmer_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class RestaurantFarmer : public SingleSwitchProgramInstance{
public:
    RestaurantFarmer();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    void run_lobby(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void run_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
};





}
}
}
#endif
