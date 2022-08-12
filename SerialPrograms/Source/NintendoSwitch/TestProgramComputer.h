/*  Test Program (Computer)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Computer_TestProgram_H
#define PokemonAutomation_Computer_TestProgram_H

#include "ComputerPrograms/ComputerProgram.h"

namespace PokemonAutomation{


class TestProgramComputer_Descriptor : public ComputerProgramDescriptor{
public:
    TestProgramComputer_Descriptor();
};



class TestProgramComputer : public ComputerProgramInstance{
public:
    TestProgramComputer();

    virtual void program(ProgramEnvironment& env, CancellableScope& scope) override;

private:

};




}
#endif
