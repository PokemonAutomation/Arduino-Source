/*  Unit Test Runner
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/ScopeExit.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "UnitTestRunner.h"

#include "CommonTools/OCR/OCR_Tests.h"
#include "PokemonHome/PokemonHome_Tests.h"
#include "PokemonSwSh/PokemonSwSh_Tests.h"
#include "PokemonLA/PokemonLA_Tests.h"
#include "PokemonSV/PokemonSV_Tests.h"
#include "PokemonLZA/PokemonLZA_Tests.h"

namespace PokemonAutomation{
namespace ComputerPrograms{






UnitTestDatabase make_UNIT_TESTS_ALL(){
    UnitTestDatabase ret;

    OCR::add_tests(ret);
    NintendoSwitch::PokemonHome::add_tests(ret);
    NintendoSwitch::PokemonSwSh::add_tests(ret);
    NintendoSwitch::PokemonLA::add_tests(ret);
    NintendoSwitch::PokemonSV::add_tests(ret);
    NintendoSwitch::PokemonLZA::add_tests(ret);

    return ret;
}




const UnitTestDatabase& UNIT_TESTS_ALL(){
    static UnitTestDatabase database = make_UNIT_TESTS_ALL();
    return database;
}
StringSelectDatabase make_TEST_DATABASE(){
    StringSelectDatabase database;
    for (auto& test : UNIT_TESTS_ALL()){
        database.add_entry(StringSelectEntry(test.first, test.first));
    }
    return database;
}
const StringSelectDatabase& TEST_DATABASE(){
    static StringSelectDatabase DATABASE = make_TEST_DATABASE();
    return DATABASE;
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





UnitTestRunner::~UnitTestRunner(){
    SUBSTRING_TEST.remove_listener(*this);
    RUN_MODE.remove_listener(*this);
}
UnitTestRunner::UnitTestRunner()
    : RESOURCE_LABEL("<b>Test Resources Path:</b>")
    , RESOURCE_PATH(
        false,
        LockMode::READ_ONLY,
        UNIT_TEST_RESOURCE_PATH(),
        ""
    )
    , RUN_MODE(
        "<b>Run Mode:</b>",
        {
            {RunMode::RUN_ONE,              "run-one",              "Run One Test"},
            {RunMode::RUN_SUBSTRING_MATCH,  "run-substring-match",  "Run Matching Substrings"},
            {RunMode::RUN_ALL,              "run-all",              "Run All Tests"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        RunMode::RUN_SUBSTRING_MATCH
    )
    , SINGLE_TEST_LABEL("<b>Test Name:</b>")
    , SINGLE_TEST(
        TEST_DATABASE(),
        LockMode::LOCK_WHILE_RUNNING,
        0
    )
    , SUBSTRING_TEST_LABEL("<b>Run All Tests Containing This Substring:</b>")
    , SUBSTRING_TEST(false, LockMode::LOCK_WHILE_RUNNING, "", "PokemonSwSh")
    , TESTS_TO_RUN(
        "<b>Tests to Run:</b>",
        LockMode::READ_ONLY,
        "", ""
    )
    , PASSED_TESTS(
        "<b>Passing Tests:</b>",
        LockMode::READ_ONLY,
        "", ""
    )
    , FAILED_TESTS(
        "<b>Failing Tests:</b>",
        LockMode::READ_ONLY,
        "", ""
    )
{
    PA_ADD_STATIC(RESOURCE_LABEL);
    PA_ADD_STATIC(RESOURCE_PATH);
    PA_ADD_OPTION(RUN_MODE);
    PA_ADD_STATIC(SINGLE_TEST_LABEL);
    PA_ADD_OPTION(SINGLE_TEST);
    PA_ADD_STATIC(SUBSTRING_TEST_LABEL);
    PA_ADD_OPTION(SUBSTRING_TEST);
    PA_ADD_STATIC(TESTS_TO_RUN);
    PA_ADD_OPTION(PASSED_TESTS);
    PA_ADD_OPTION(FAILED_TESTS);

    UnitTestRunner::on_config_value_changed(this);

    RUN_MODE.add_listener(*this);
    SUBSTRING_TEST.add_listener(*this);
}
void UnitTestRunner::on_config_value_changed(void* object){
    if (object == &RUN_MODE){
        SINGLE_TEST_LABEL.set_visibility(ConfigOptionState::HIDDEN);
        SINGLE_TEST.set_visibility(ConfigOptionState::HIDDEN);
        SUBSTRING_TEST_LABEL.set_visibility(ConfigOptionState::HIDDEN);
        SUBSTRING_TEST.set_visibility(ConfigOptionState::HIDDEN);
        TESTS_TO_RUN.set_visibility(ConfigOptionState::HIDDEN);

        switch ((RunMode)RUN_MODE){
        case RunMode::RUN_ONE:
            SINGLE_TEST_LABEL.set_visibility(ConfigOptionState::ENABLED);
            SINGLE_TEST.set_visibility(ConfigOptionState::ENABLED);
            break;
        case RunMode::RUN_SUBSTRING_MATCH:
            SUBSTRING_TEST_LABEL.set_visibility(ConfigOptionState::ENABLED);
            SUBSTRING_TEST.set_visibility(ConfigOptionState::ENABLED);
            TESTS_TO_RUN.set_visibility(ConfigOptionState::ENABLED);
            break;
        case RunMode::RUN_ALL:
            break;
        }
        return;
    }
    if (object == &SUBSTRING_TEST){
        std::string name = SUBSTRING_TEST;
        TESTS_TO_RUN.set("");
        size_t count = 0;
        for (const auto& test : UNIT_TESTS_ALL()){
            if (test.first.contains(name)){
                count++;
                TESTS_TO_RUN.append(test.first + "\n");
            }
        }
        TESTS_TO_RUN.set_label("<b>Tests to Run: " + tostr_u_commas(count) + "</b>");
    }
}


void UnitTestRunner::program(ProgramEnvironment& env, CancellableScope& scope){
    PASSED_TESTS.set("");
    FAILED_TESTS.set("");

    m_env = &env;
    ScopeExit run_on_exit([this]{
        m_env = nullptr;
    });

    PokemonAutomation::UnitTestRunner runner(
        &scope,
        env.logger(),
        GlobalThreadPools::computation_normal()
    );
    runner.add_listener(*this);


    const UnitTestDatabase& all_tests = UNIT_TESTS_ALL();

    switch ((RunMode)RUN_MODE){
    case RunMode::RUN_ONE:{
        auto iter = all_tests.find(SINGLE_TEST.slug());
        if (iter == all_tests.end()){
            throw InternalProgramError(
                &env.logger(),
                PA_CURRENT_FUNCTION,
                "Test does not exist: " + SINGLE_TEST.display_name()
            );
        }
        runner.add_test(iter->second);
        break;
    }
    case RunMode::RUN_SUBSTRING_MATCH:{
        std::string name = SUBSTRING_TEST;
        for (const auto& test : all_tests){
            if (test.first.contains(name)){
                runner.add_test(test.second);
            }
        }
        break;
    }
    case RunMode::RUN_ALL:{
        for (const auto& test : all_tests){
            runner.add_test(test.second);
        }
        break;
    }
    }

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
        PASSED_TESTS.append(test->name() + "\n");
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






bool CommandLineUnitTestRunner::run(){
    PokemonAutomation::UnitTestRunner runner(
        m_logger,
        GlobalThreadPools::computation_normal()
    );
    runner.add_listener(*this);
    for (const auto& test : UNIT_TESTS_ALL()){
        runner.add_test(test.second);
    }
    runner.run();

    m_logger.log(
        "Tests Finished:"
        "\n    Passed:  " + tostr_u_commas(m_passed_tests.load(std::memory_order_acquire)) +
        "\n    Failed:  " + tostr_u_commas(m_failed_tests.load(std::memory_order_acquire)) +
        "\n    Skipped: " + tostr_u_commas(m_skipped_tests.load(std::memory_order_acquire))
    );
    return m_failed_tests.load(std::memory_order_acquire) != 0;
}
void CommandLineUnitTestRunner::on_test_finished(
    std::shared_ptr<const UnitTest> test,
    UnitTestResult result
){
    switch (result.result){
    case UnitTestResult::NOT_RUN:
        m_logger.log("NOT RUN: " + test->name(), COLOR_ORANGE);
        m_skipped_tests++;
        break;
    case UnitTestResult::PASSED:
        m_logger.log("PASSED: " + test->name(), COLOR_BLUE);
        m_passed_tests++;
        break;
    case UnitTestResult::FAILED:
        m_logger.log("FAILED: " + test->name(), COLOR_RED);
        m_failed_tests++;
        break;
    case UnitTestResult::SKIPPED:
        m_logger.log("SKIPPED: " + test->name(), COLOR_ORANGE);
        m_skipped_tests++;
        break;
    case UnitTestResult::OOM:
        m_logger.log("OOM: " + test->name(), COLOR_RED);
        m_failed_tests++;
        break;
    }
}











}
}
