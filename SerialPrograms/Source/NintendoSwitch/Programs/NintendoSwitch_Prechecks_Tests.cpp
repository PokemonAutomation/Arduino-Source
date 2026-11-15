/*  Prechecks - Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  run_prechecks() with fake checks: no console, no GUI.
 */

#include <format>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "NintendoSwitch_Prechecks.h"
#include "NintendoSwitch_Prechecks_Tests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

namespace{


//  A check that only has a name and a policy. The tests run it through
//  run_check, never through run().
class FakePrecheck : public Precheck{
public:
    using Precheck::Precheck;
    virtual PrecheckOutcome run(SingleSwitchProgramEnvironment&, ProControllerContext&) const override{
        return {PrecheckStatus::FAILED, "A fake check must not touch the console."};
    }
};

struct FakeCheck{
    std::string name;
    PrecheckPolicy policy;
    PrecheckOutcome outcome;
};

struct RunPrechecksInput{
    std::vector<FakeCheck> checks;
    SkippablePrechecks skippable;
};

struct RunPrechecksExpectation{
    //  Names of the checks run_prechecks() ran, in order.
    std::vector<std::string> ran;
    //  Set when run_prechecks() throws UserSetupError: its message.
    std::optional<std::string> error;
    //  Statuses run_prechecks() returned, in order, when it did not throw.
    std::vector<PrecheckStatus> statuses;
};

struct RunPrechecksCase{
    std::string name;
    RunPrechecksInput input;
    RunPrechecksExpectation expectation;
};


//  Runs the case's fake checks; returns what run_prechecks() did.
RunPrechecksExpectation run_fake_prechecks(Logger& logger, const RunPrechecksInput& input){
    PrecheckList checks;
    std::map<std::string, PrecheckOutcome> outcomes;
    for (const FakeCheck& check : input.checks){
        checks.emplace_back(std::make_unique<FakePrecheck>(check.name, "Fake needs.", check.policy));
        outcomes.emplace(check.name, check.outcome);
    }
    RunPrechecksExpectation actual;
    auto run_check = [&](const Precheck& check){
        actual.ran.emplace_back(check.name());
        return outcomes.at(check.name());
    };
    try{
        for (const PrecheckResult& result : run_prechecks(logger, checks, input.skippable, run_check)){
            actual.statuses.emplace_back(result.outcome.status);
        }
    }catch (UserSetupError& e){
        actual.error = e.message();
    }
    return actual;
}


class Test_RunPrechecks : public UnitTest{
public:
    explicit Test_RunPrechecks(RunPrechecksCase test_case)
        : UnitTest(std::format("NintendoSwitch::Prechecks::RunPrechecks - {}", test_case.name))
        , m_case(std::move(test_case))
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope&) const override{
        const RunPrechecksExpectation actual = run_fake_prechecks(logger, m_case.input);
        const RunPrechecksExpectation& expected = m_case.expectation;
        if (actual.ran != expected.ran){
            return "run_prechecks() ran the wrong checks or in the wrong order.";
        }
        if (actual.error != expected.error){
            return std::format("run_prechecks() threw the wrong error: \"{}\"", actual.error.value_or("(none)"));
        }
        if (actual.statuses != expected.statuses){
            return "run_prechecks() returned the wrong statuses.";
        }
        return true;
    }

private:
    RunPrechecksCase m_case;
};


using enum PrecheckPolicy;
using enum PrecheckStatus;

const PrecheckOutcome PASS{PASSED, ""};
const PrecheckOutcome PARTY_FULL{FAILED, "Your party is full."};
const PrecheckOutcome NO_PARTY_MENU{SKIPPED, "Unable to open the party menu."};

const std::vector<RunPrechecksCase> RUN_PRECHECKS_CASES{
    {
        .name = "AllPassInOrder",
        .input = {{{"first", ALWAYS_RUNS, PASS}, {"second", SKIPPABLE, PASS}}, SkippablePrechecks::RUN},
        .expectation = {{"first", "second"}, std::nullopt, {PASSED, PASSED}},
    },
    {
        .name = "SkipLeavesOutSkippableChecks",
        .input = {{{"first", SKIPPABLE, PASS}, {"second", ALWAYS_RUNS, PASS}}, SkippablePrechecks::SKIP},
        .expectation = {{"second"}, std::nullopt, {SKIPPED, PASSED}},
    },
    {
        .name = "FirstFailureStopsWithItsMessage",
        .input = {{{"first", ALWAYS_RUNS, PARTY_FULL}, {"second", ALWAYS_RUNS, PASS}}, SkippablePrechecks::RUN},
        .expectation = {{"first"}, "Your party is full.", {}},
    },
    {
        .name = "SkippedCheckDoesNotStopTheRun",
        .input = {{{"first", SKIPPABLE, NO_PARTY_MENU}, {"second", ALWAYS_RUNS, PASS}}, SkippablePrechecks::RUN},
        .expectation = {{"first", "second"}, std::nullopt, {SKIPPED, PASSED}},
    },
    {
        .name = "EmptyListRunsNothing",
        .input = {{}, SkippablePrechecks::RUN},
        .expectation = {{}, std::nullopt, {}},
    },
};


}  // namespace


void add_tests_Prechecks(UnitTestDatabase& database){
    for (const RunPrechecksCase& test_case : RUN_PRECHECKS_CASES){
        database.add<Test_RunPrechecks>(test_case);
    }
}


}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
