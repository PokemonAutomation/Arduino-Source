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


class EggFetcher2_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    EggFetcher2_Descriptor();
};



class EggFetcher2 : public SingleSwitchProgramInstance{
public:
    EggFetcher2(const EggFetcher2_Descriptor& descriptor);

    void run_eggfetcher(SingleSwitchProgramEnvironment& env, bool deposit_automatically, uint16_t attempts) const;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    SimpleInteger<uint16_t> MAX_FETCH_ATTEMPTS;
};



}
}
}
#endif
