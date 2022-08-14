/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



class ShinyEncounterTester_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyEncounterTester_Descriptor();
};



class ShinyEncounterTester : public SingleSwitchProgramInstance{
public:
    ShinyEncounterTester();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    EnumDropdownOption ENCOUNTER_TYPE;

    BooleanCheckBoxOption USE_SOUND_DETECTION;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
