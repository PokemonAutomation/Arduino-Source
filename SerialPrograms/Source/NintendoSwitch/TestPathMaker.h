#ifndef TESTPATHMAKER_H
#define TESTPATHMAKER_H

#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/TestPathMakerTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class TestPathMaker_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    TestPathMaker_Descriptor();
};


class TestPathMaker : public SingleSwitchProgramInstance{
public:
    TestPathMaker(const TestPathMaker_Descriptor& descriptor);

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
