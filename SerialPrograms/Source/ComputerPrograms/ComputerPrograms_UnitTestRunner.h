/*  Unit Test Runner
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ComputerPrograms_UnitTestRunner_H
#define PokemonAutomation_ComputerPrograms_UnitTestRunner_H

#include <atomic>
#include <span>
#include <string>
#include <string_view>
#include <vector>
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
    using Descriptor = UnitTestRunner_Descriptor;

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
        RUN_ONE,
        RUN_SUBSTRING_MATCH,
        RUN_ALL,
    };
    EnumDropdownOption<RunMode> RUN_MODE;

    StaticTextOption SINGLE_TEST_LABEL;
    StringSelectCell SINGLE_TEST;

    StaticTextOption SUBSTRING_TEST_LABEL;
    StringCell SUBSTRING_TEST;

    TextEditOption TESTS_TO_RUN;
    TextEditOption PASSED_TESTS;
    TextEditOption FAILED_TESTS;

    ProgramEnvironment* m_env = nullptr;
    std::atomic<uint64_t> m_total_failed;
};



class CommandLineUnitTestRunner : public PokemonAutomation::UnitTestRunner::Listener{
public:
    CommandLineUnitTestRunner(Logger& logger)
        : m_logger(logger)
        , m_skipped_tests(0)
        , m_passed_tests(0)
        , m_failed_tests(0)
    {}

    //  Run the unit tests whose names contain any of `filters`, or all of them if `filters` is
    //  empty. Return the number of failed tests.
    //  If `augment_color_distortions` is true, also run each image test under different color
    //  distortion augmentation to widen the test coverage, using the distortion data in the
    //  CaptureCardCalibration/ folder of the test repo. If that data can't be loaded, log the
    //  error and return 1 without running any test.
    size_t run(bool augment_color_distortions, std::span<const std::string_view> filters);


private:
    virtual void on_test_finished(
        std::shared_ptr<const UnitTest> test,
        UnitTestResult result
    ) override;


private:
    Logger& m_logger;
    std::atomic<size_t> m_skipped_tests;
    std::atomic<size_t> m_passed_tests;
    std::atomic<size_t> m_failed_tests;
};


//  The names of the unit tests that contain any of `filters`, or of all the tests if `filters`
//  is empty, in name order.
std::vector<std::string> unit_test_names(std::span<const std::string_view> filters);




}
}
#endif
