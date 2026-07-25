/*  Unit Test Runner
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ComputerPrograms_UnitTestRunner_H
#define PokemonAutomation_ComputerPrograms_UnitTestRunner_H

#include "Common/Cpp/TestRunners/ParallelUnitTestRunner.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/TextEditOption.h"
#include "CommonTools/Options/StringSelectOption.h"
#include "ComputerPrograms/ComputerProgram.h"

namespace PokemonAutomation{
namespace ComputerPrograms{


class UnitTestRunner_Descriptor : public ComputerProgramDescriptor{
public:
    UnitTestRunner_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class UnitTestRunner
    : public ComputerProgramInstance
    , public PokemonAutomation::UnitTestRunner::Listener
    , public ConfigOption::Listener
{
public:
    ~UnitTestRunner();
    UnitTestRunner();
    virtual void program(ProgramEnvironment& env, CancellableScope& scope) override;


private:
    virtual void on_config_value_changed(void* object) override;

    virtual void on_test_finished(
        std::shared_ptr<const UnitTest> test,
        UnitTestResult result
    ) override;


private:
    StaticTextOption RESOURCE_LABEL;
    StringCell RESOURCE_PATH;

    enum class RunMode{
        RUN_ALL,
        RUN_ONE,
    };
    EnumDropdownOption<RunMode> RUN_MODE;

    StaticTextOption SINGLE_TEST_LABEL;
    StringSelectCell SINGLE_TEST;

    TextEditOption PASSED_TESTS;
    TextEditOption FAILED_TESTS;

    ProgramEnvironment* m_env = nullptr;
};



}
}
#endif
