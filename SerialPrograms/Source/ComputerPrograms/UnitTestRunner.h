/*  Unit Test Runner
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ComputerPrograms_UnitTestRunner_H
#define PokemonAutomation_ComputerPrograms_UnitTestRunner_H

#include "Common/Cpp/TestRunners/ParallelUnitTestRunner.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/TextEditOption.h"
#include "ComputerPrograms/ComputerProgram.h"

namespace PokemonAutomation{
namespace ComputerPrograms{


class UnitTestRunner_Descriptor : public ComputerProgramDescriptor{
public:
    UnitTestRunner_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class UnitTestRunner : public ComputerProgramInstance, public PokemonAutomation::UnitTestRunner::Listener{
public:
    UnitTestRunner();
    virtual void program(ProgramEnvironment& env, CancellableScope& scope) override;


private:
    virtual void on_test_finished(
        std::shared_ptr<const UnitTest> test,
        UnitTestResult result
    ) override;


private:
    StringOption RESOURCE_PATH;
    TextEditOption PASSED_TESTS;
    TextEditOption FAILED_TESTS;

    ProgramEnvironment* m_env = nullptr;
};







}
}
#endif
