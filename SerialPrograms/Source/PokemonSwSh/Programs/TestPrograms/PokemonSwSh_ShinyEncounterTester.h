/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



class ShinyEncounterTester_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyEncounterTester_Descriptor();
};



class ShinyEncounterTester : public SingleSwitchProgramInstance{
public:
    ShinyEncounterTester();

    virtual void start_program_controller_check(ControllerSession& session) override{}
    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    enum class EncounterType{
        Wild,
        Raid,
    };
    EnumDropdownOption<EncounterType> ENCOUNTER_TYPE;
};




}
}
}
