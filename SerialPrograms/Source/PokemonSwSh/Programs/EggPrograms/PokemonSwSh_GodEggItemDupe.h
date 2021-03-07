/*  God Egg Item Duplication
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_GodEggItemDupe_H
#define PokemonAutomation_PokemonSwSh_GodEggItemDupe_H

#include "CommonFramework/Options/BooleanCheckBox.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh_EggHelpers.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class GodEggItemDupe : public SingleSwitchProgram{
public:
    GodEggItemDupe();

    void collect_godegg(uint8_t party_slot, bool map_to_pokemon, bool pokemon_to_map) const;
    void run_program(SingleSwitchProgramEnvironment& env, uint16_t attempts) const;
    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    SimpleInteger<uint16_t> MAX_FETCH_ATTEMPTS;
    SimpleInteger<uint8_t> PARTY_ROUND_ROBIN;
    BooleanCheckBox DETACH_BEFORE_RELEASE;
};


}
}
}
#endif
