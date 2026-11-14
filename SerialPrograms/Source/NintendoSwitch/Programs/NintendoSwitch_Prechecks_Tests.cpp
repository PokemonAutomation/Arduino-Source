/*  Prechecks - Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  run_prechecks() with fake checks: no console, no GUI.
 */

#include <map>
#include <optional>
#include <string>
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

struct RunPrechecksCase{
    struct Input{
        std::vector<FakeCheck> checks;
        OptionalPrechecks optional;
    };
    struct Expectation{
        //  Names of the checks run_prechecks() ran, in order.
        std::vector<std::string> ran;
        //  Set when run_prechecks() throws UserSetupError: its message.
        std::optional<std::string> error;
        //  Statuses run_prechecks() returned, in order, when it did not throw.
        std::vector<PrecheckStatus> statuses;
    };
    std::string name;
    Input input;
    Expectation expectation;
};

const PrecheckOutcome PASS{PrecheckStatus::PASSED, ""};


class Test_RunPrechecks : public UnitTest{
public:
    explicit Test_RunPrechecks(RunPrechecksCase test_case)
        : UnitTest("NintendoSwitch::Prechecks::RunPrechecks - " + test_case.name)
        , m_case(std::move(test_case))
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope&) const override{
        PrecheckList checks;
        std::map<std::string, PrecheckOutcome> outcomes;
        for (const FakeCheck& check : m_case.input.checks){
            checks.emplace_back(std::make_unique<FakePrecheck>(check.name, "Fake needs.", check.policy));
            outcomes.emplace(check.name, check.outcome);
        }
        std::vector<std::string> ran;
        auto run_check = [&](const Precheck& check){
            ran.emplace_back(check.name());
            return outcomes.at(check.name());
        };

        const RunPrechecksCase::Expectation& expected = m_case.expectation;
        std::optional<std::string> error;
        std::vector<PrecheckStatus> statuses;
        try{
            for (const PrecheckResult& result : run_prechecks(logger, checks, m_case.input.optional, run_check)){
                statuses.emplace_back(result.outcome.status);
            }
        }catch (UserSetupError& e){
            error = e.message();
        }

        if (ran != expected.ran){
            return "run_prechecks() ran the wrong checks or in the wrong order.";
        }
        if (error != expected.error){
            return "run_prechecks() threw the wrong error: \"" + error.value_or("(none)") + "\"";
        }
        if (statuses != expected.statuses){
            return "run_prechecks() returned the wrong statuses.";
        }
        return true;
    }

private:
    RunPrechecksCase m_case;
};


std::vector<RunPrechecksCase> run_prechecks_cases(){
    using enum PrecheckPolicy;
    using enum PrecheckStatus;
    const PrecheckOutcome no_room{FAILED, "Your party is full."};
    const PrecheckOutcome no_menu{SKIPPED, "Unable to open the party menu."};
    return {
        {
            "AllPassInOrder",
            {{{"first", REQUIRED, PASS}, {"second", OPTIONAL, PASS}}, OptionalPrechecks::RUN},
            {{"first", "second"}, std::nullopt, {PASSED, PASSED}},
        },
        {
            "SkipLeavesOutOptionalChecks",
            {{{"first", OPTIONAL, PASS}, {"second", REQUIRED, PASS}}, OptionalPrechecks::SKIP},
            {{"second"}, std::nullopt, {SKIPPED, PASSED}},
        },
        {
            "FirstFailureStopsWithItsMessage",
            {{{"first", REQUIRED, no_room}, {"second", REQUIRED, PASS}}, OptionalPrechecks::RUN},
            {{"first"}, "Your party is full.", {}},
        },
        {
            "SkippedCheckDoesNotStopTheRun",
            {{{"first", OPTIONAL, no_menu}, {"second", REQUIRED, PASS}}, OptionalPrechecks::RUN},
            {{"first", "second"}, std::nullopt, {SKIPPED, PASSED}},
        },
        {
            "EmptyListRunsNothing",
            {{}, OptionalPrechecks::RUN},
            {{}, std::nullopt, {}},
        },
    };
}


}  // namespace


void add_tests_Prechecks(UnitTestDatabase& database){
    for (RunPrechecksCase& test_case : run_prechecks_cases()){
        database.add<Test_RunPrechecks>(std::move(test_case));
    }
}


}
}
