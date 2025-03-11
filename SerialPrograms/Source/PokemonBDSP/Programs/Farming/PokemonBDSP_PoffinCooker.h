/*  Poffin Cooker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_PoffinCooker_H
#define PokemonAutomation_PokemonBDSP_PoffinCooker_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class PoffinCooker_Descriptor : public SingleSwitchProgramDescriptor{
public:
    PoffinCooker_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class PoffinCooker : public SingleSwitchProgramInstance{
public:
    PoffinCooker();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    SimpleIntegerOption<uint16_t> MAX_COOK_ATTEMPTS;

};




}
}
}
#endif
