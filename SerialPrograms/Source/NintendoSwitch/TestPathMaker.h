/*  Test Program (Computer)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Computer_TestPathMaker_H
#define PokemonAutomation_Computer_TestPathMaker_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/TestPathMakerTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class TestPathMaker_Descriptor : public SingleSwitchProgramDescriptor{
public:
    TestPathMaker_Descriptor();
};


class TestPathMaker : public SingleSwitchProgramInstance{
public:
    TestPathMaker();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    void run_path(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void execute_action(ConsoleHandle& console, BotBaseContext& context, size_t action_index);

private:
    TestPathMakerTable PATH;
};



}
}
#endif // TESTPATHMAKER_H
