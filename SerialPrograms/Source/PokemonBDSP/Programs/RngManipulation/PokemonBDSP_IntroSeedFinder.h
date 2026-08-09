/*  BDSP Intro Seed Finder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_IntroSeedFinder_H
#define PokemonAutomation_PokemonBDSP_IntroSeedFinder_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonBDSP_RngDisplays.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class IntroSeedFinder_Descriptor : public SingleSwitchProgramDescriptor{
public:
    IntroSeedFinder_Descriptor();
};


class IntroSeedFinder : public SingleSwitchProgramInstance{
public:
    IntroSeedFinder();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    BlinkCollectionDisplay COLLECTION_DISPLAY;
    RngStateDisplay STATE_DISPLAY;
};


}
}
}
#endif
