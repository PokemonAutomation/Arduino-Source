/*  God Egg Duplication
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_GodEggDuplication_H
#define PokemonAutomation_PokemonSwSh_GodEggDuplication_H

#include "CommonFramework/Options/BooleanCheckBox.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh_EggHelpers.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class GodEggDuplication : public SingleSwitchProgram{
public:
    GodEggDuplication();

    void collect_godegg(const BotBaseContext& context, uint8_t party_slot) const;
    void run_program(SingleSwitchProgramEnvironment& env, uint16_t attempts) const;
    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    SimpleInteger<uint16_t> MAX_FETCH_ATTEMPTS;
    SimpleInteger<uint8_t> PARTY_ROUND_ROBIN;
};


}
}
}
#endif
