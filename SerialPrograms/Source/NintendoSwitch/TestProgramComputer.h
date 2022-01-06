/*  Test Program (Computer)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Computer_TestProgram_H
#define PokemonAutomation_Computer_TestProgram_H

#include "CommonFramework/Panels/RunnableComputerProgram.h"

namespace PokemonAutomation{


class TestProgramComputer_Descriptor : public RunnableComputerProgramDescriptor{
public:
    TestProgramComputer_Descriptor();
};



class TestProgramComputer : public RunnableComputerProgramInstance{
public:
    TestProgramComputer(const TestProgramComputer_Descriptor& descriptor);

    virtual void program(ProgramEnvironment& env) override;

private:

};




}
#endif
