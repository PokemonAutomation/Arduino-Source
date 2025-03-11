/*  Apply Grits
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_ApplyGrits_H
#define PokemonAutomation_PokemonLA_ApplyGrits_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class ApplyGrits_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ApplyGrits_Descriptor();
};


class ApplyGrits : public SingleSwitchProgramInstance{
public:
    ApplyGrits();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:

    void ApplyGritsOnOnePokemon(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t pokemon_index);

    IntegerEnumDropdownOption NUM_POKEMON;

    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
