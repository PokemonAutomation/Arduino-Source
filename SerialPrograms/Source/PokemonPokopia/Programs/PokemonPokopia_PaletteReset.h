/*  Palette Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonPokopia_PaletteReset_H
#define PokemonAutomation_PokemonPokopia_PaletteReset_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/ButtonOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{

template <typename Type> class ControllerContext;

namespace NintendoSwitch{

class ProController;
using ProControllerContext = ControllerContext<ProController>;

namespace PokemonPokopia{


class PaletteReset_Descriptor : public SingleSwitchProgramDescriptor{
public:
    PaletteReset_Descriptor();

};


class PaletteReset : public SingleSwitchProgramInstance{
public:
    PaletteReset();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;


private:
    SimpleIntegerOption<uint32_t> SKIPS;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationsOption NOTIFICATIONS;
};



}
}
}
#endif
