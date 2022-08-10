/*  Auto Multi-Spawn
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *  
 *  Advance a path in Multi-Spawn shiny hunting method
 */

#ifndef PokemonAutomation_PokemonLA_AutoMultiSpawn_H
#define PokemonAutomation_PokemonLA_AutoMultiSpawn_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "CommonFramework/Options/StringOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class AutoMultiSpawn_Descriptor : public SingleSwitchProgramDescriptor{
public:
    AutoMultiSpawn_Descriptor();
};


class AutoMultiSpawn : public SingleSwitchProgramInstance2{
public:
    AutoMultiSpawn();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:

    EnumDropdownOption SPAWN;

    StringOption PATH;

    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
