/*  Egg Fetcher 2
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EggFetcher2_H
#define PokemonAutomation_PokemonSwSh_EggFetcher2_H

#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh_EggHelpers.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class EggFetcher2 : public SingleSwitchProgram{
public:
    EggFetcher2();

    void run_eggfetcher(SingleSwitchProgramEnvironment& env, bool deposit_automatically, uint16_t attempts) const;
    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    SimpleInteger<uint16_t> MAX_FETCH_ATTEMPTS;
};



}
}
}
#endif
