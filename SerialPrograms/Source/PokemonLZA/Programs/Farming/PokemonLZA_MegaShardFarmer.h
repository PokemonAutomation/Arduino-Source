/*  Mega Shard Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_MegaShardFarmer_H
#define PokemonAutomation_PokemonLZA_MegaShardFarmer_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


class MegaShardFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    MegaShardFarmer_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class MegaShardFarmer : public SingleSwitchProgramInstance{
public:
    MegaShardFarmer();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    void fly_back(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    BooleanCheckBoxOption SKIP_SHARDS;
};





}
}
}
#endif
