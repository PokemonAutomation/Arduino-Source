/*  BDSP Bedroom Seed Finder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BedroomSeedFinder_H
#define PokemonAutomation_PokemonBDSP_BedroomSeedFinder_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonBDSP/Options/PokemonBDSP_PlayerModelOption.h"
#include "PokemonBDSP_RngDisplays.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class BedroomSeedFinder_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BedroomSeedFinder_Descriptor();
};


class BedroomSeedFinder : public SingleSwitchProgramInstance{
public:
    BedroomSeedFinder();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    PlayerModelOption PLAYER_MODEL;

    BlinkCollectionDisplay COLLECTION_DISPLAY;
    RngStateDisplay STATE_DISPLAY;
};


}
}
}
#endif
