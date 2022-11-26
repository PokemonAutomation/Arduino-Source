/*  Fast Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_FastCodeEntry_H
#define PokemonAutomation_PokemonSV_FastCodeEntry_H

#include "Common/Cpp/Options/StringOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class FastCodeEntry_Descriptor : public SingleSwitchProgramDescriptor{
public:
    FastCodeEntry_Descriptor();
};




class FastCodeEntry : public SingleSwitchProgramInstance{
public:
    FastCodeEntry();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    StringOption CODE;
};




}
}
}
#endif
