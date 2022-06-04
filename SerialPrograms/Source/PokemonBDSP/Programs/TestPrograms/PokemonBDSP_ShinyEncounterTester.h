/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



class ShinyEncounterTester_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ShinyEncounterTester_Descriptor();
};



class ShinyEncounterTester : public SingleSwitchProgramInstance{
public:
    ShinyEncounterTester(const ShinyEncounterTester_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    EnumDropdownOption ENCOUNTER_TYPE;

    BooleanCheckBoxOption USE_SOUND_DETECTION;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
