/*  Unit Test Runner
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/ScopeExit.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "UnitTestRunner.h"

#include "PokemonSwSh/Inference/PokemonSwSh_BoxGenderDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_YCommDetector.h"

namespace PokemonAutomation{
namespace ComputerPrograms{




void add_tests(PokemonAutomation::UnitTestRunner& runner){



    for (auto& test : NintendoSwitch::PokemonSwSh::get_tests_BoxGenderDetector()){
        runner.add_test(test);
    }
    for (auto& test : NintendoSwitch::PokemonSwSh::get_tests_YCommDetector()){
        runner.add_test(test);
    }



}









UnitTestRunner_Descriptor::UnitTestRunner_Descriptor()
    : ComputerProgramDescriptor(
        "UnitTestRunner",
        "Nintendo Switch",
        "Unit Test Runner",
        "",
        "UI to run unit tests."
    )
{}
class UnitTestRunner_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : tests(m_stats["Tests"])
        , passed(m_stats["Passed"])
        , failed(m_stats["Failed"])
        , skipped(m_stats["Skipped"])
        , oom(m_stats["OOM"])
    {
        m_display_order.emplace_back("Tests");
        m_display_order.emplace_back("Passed");
        m_display_order.emplace_back("Failed");
        m_display_order.emplace_back("Skipped");
        m_display_order.emplace_back("OOM");
    }

    std::atomic<uint64_t>& tests;
    std::atomic<uint64_t>& passed;
    std::atomic<uint64_t>& failed;
    std::atomic<uint64_t>& skipped;
    std::atomic<uint64_t>& oom;
};
std::unique_ptr<StatsTracker> UnitTestRunner_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




UnitTestRunner::UnitTestRunner()
    : RESOURCE_LABEL("<b>Test Resources Path:</b>")
    , RESOURCE_PATH(
        false,
        LockMode::READ_ONLY,
        UNIT_TEST_RESOURCE_PATH(),
        ""
    )
    , PASSED_TESTS(
        "<b>Passing Tests</b>",
        LockMode::READ_ONLY,
        "", ""
    )
    , FAILED_TESTS(
        "<b>Failing Tests</b>",
        LockMode::READ_ONLY,
        "", ""
    )
{
    PA_ADD_STATIC(RESOURCE_LABEL);
    PA_ADD_STATIC(RESOURCE_PATH);
    PA_ADD_OPTION(PASSED_TESTS);
    PA_ADD_OPTION(FAILED_TESTS);
}
void UnitTestRunner::program(ProgramEnvironment& env, CancellableScope& scope){
    PASSED_TESTS.set("");
    FAILED_TESTS.set("");

    m_env = &env;
    ScopeExit run_on_exit([this]{
        m_env = nullptr;
    });

    PokemonAutomation::UnitTestRunner runner(env.logger(), GlobalThreadPools::computation_normal());
    runner.add_listener(*this);

    add_tests(runner);

    runner.run();
}



void UnitTestRunner::on_test_finished(
    std::shared_ptr<const UnitTest> test,
    UnitTestResult result
){
    if (m_env == nullptr){
        return;
    }
    UnitTestRunner_Descriptor::Stats& stats = m_env->current_stats<UnitTestRunner_Descriptor::Stats>();
    stats.tests++;
    switch (result.result){
    case UnitTestResult::NOT_RUN:
        stats.skipped++;
        break;
    case UnitTestResult::PASSED:
        stats.passed++;
        PASSED_TESTS.append("PASSED: " + test->name() + "\n");
        break;
    case UnitTestResult::FAILED:
        stats.failed++;
        FAILED_TESTS.append("FAILED: " + test->name() + "\n");
        break;
    case UnitTestResult::SKIPPED:
        stats.skipped++;
        break;
    case UnitTestResult::OOM:
        stats.oom++;
        FAILED_TESTS.append("OOM: " + test->name() + "\n");
        break;
    }
    m_env->update_stats();
}















}
}
