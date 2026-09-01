/*  Pokemon Home Test Database Generator
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_TestDatabaseGenerator_H
#define PokemonAutomation_PokemonHome_TestDatabaseGenerator_H

#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


class TestDatabaseGenerator_Descriptor : public SingleSwitchProgramDescriptor{
public:
    TestDatabaseGenerator_Descriptor();
};

class TestDatabaseGenerator : public SingleSwitchProgramInstance{
public:
    TestDatabaseGenerator();
    virtual void start_program_controller_check(ControllerSession& session) override{}
    virtual void program(
        SingleSwitchProgramEnvironment& env,
        CancellableScope& scope
    ) override;

private:
    StringOption DIRECTORY;
    StringOption PRINT_DIRECTORY;
    BooleanCheckBoxOption RENAME_FILES;
};


}
}
}
#endif
