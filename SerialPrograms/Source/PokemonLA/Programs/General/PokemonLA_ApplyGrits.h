/*  Apply Grits
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_ApplyGrits_H
#define PokemonAutomation_PokemonLA_ApplyGrits_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class ApplyGrits_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ApplyGrits_Descriptor();
};


class ApplyGrits : public SingleSwitchProgramInstance{
public:
    ApplyGrits(const ApplyGrits_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:

    void ApplyGritsOnOnePokemon(SingleSwitchProgramEnvironment& env, BotBaseContext& context, size_t pokemon_index);

    EnumDropdownOption NUM_POKEMON;

    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
