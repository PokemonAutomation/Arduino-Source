/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"

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

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    EnumDropdownOption<BattleType> ENCOUNTER_TYPE;

    BooleanCheckBoxOption USE_SOUND_DETECTION;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
