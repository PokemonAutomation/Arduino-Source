/*  FRLG Prechecks - Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  The FRLG precheck lists and verdicts: no console, no GUI.
 */

#include <format>
#include <string>
#include <vector>
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Programs/RngManipulation/PokemonFRLG_RngProgram.h"
#include "PokemonFRLG_Prechecks.h"
#include "PokemonFRLG_Prechecks_Tests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

namespace{


struct ListedPrecheck{
    std::string name;
    PrecheckPolicy policy;

    bool operator==(const ListedPrecheck&) const = default;
};

struct RngPrechecksInput{
    size_t free_party_slots_needed;
};

struct RngPrechecksExpectation{
    std::vector<ListedPrecheck> checks;
};

struct RngPrechecksCase{
    std::string name;
    RngPrechecksInput input;
    RngPrechecksExpectation expectation;
};

class Test_RngPrechecks : public UnitTest{
public:
    explicit Test_RngPrechecks(RngPrechecksCase test_case)
        : UnitTest(std::format("PokemonFRLG::Prechecks::RngPrechecks - {}", test_case.name))
        , m_case(std::move(test_case))
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        std::vector<ListedPrecheck> listed;
        for (const std::unique_ptr<const Precheck>& check : rng_prechecks(m_case.input.free_party_slots_needed)){
            if (check->needs().empty()){
                return std::format("Precheck {} must say what it needs.", check->name());
            }
            listed.emplace_back(ListedPrecheck{check->name(), check->policy()});
        }
        if (listed != m_case.expectation.checks){
            return "The RNG precheck list has the wrong checks, order or policies.";
        }
        return true;
    }

private:
    RngPrechecksCase m_case;
};


struct FreePartySlotsInput{
    size_t party_size;
    size_t free_slots_needed;
};

struct FreePartySlotsExpectation{
    PrecheckStatus status;
    //  Checked only on FAILED: the message the user sees.
    std::string message;
};

struct FreePartySlotsCase{
    std::string name;
    FreePartySlotsInput input;
    FreePartySlotsExpectation expectation;
};

class Test_FreePartySlotsOutcome : public UnitTest{
public:
    explicit Test_FreePartySlotsOutcome(FreePartySlotsCase test_case)
        : UnitTest(std::format("PokemonFRLG::Prechecks::FreePartySlotsOutcome - {}", test_case.name))
        , m_case(std::move(test_case))
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        const PrecheckOutcome outcome = free_party_slots_outcome(
            m_case.input.party_size, m_case.input.free_slots_needed
        );
        if (outcome.status != m_case.expectation.status){
            return std::format("Wrong verdict: \"{}\"", outcome.message);
        }
        if (outcome.status == PrecheckStatus::FAILED && outcome.message != m_case.expectation.message){
            return std::format("Wrong message: \"{}\"", outcome.message);
        }
        return true;
    }

private:
    FreePartySlotsCase m_case;
};


const ListedPrecheck BLACK_BORDER{"black_border", PrecheckPolicy::ALWAYS_RUNS};
const ListedPrecheck FREE_PARTY_SLOTS{"free_party_slots", PrecheckPolicy::SKIPPABLE};

const std::vector<RngPrechecksCase> RNG_PRECHECKS_CASES{
    {.name = "NeedsNoSlots", .input = {0}, .expectation = {{BLACK_BORDER}}},
    {.name = "NeedsOneSlot", .input = {1}, .expectation = {{BLACK_BORDER, FREE_PARTY_SLOTS}}},
    {.name = "NeedsTwoSlots", .input = {2}, .expectation = {{BLACK_BORDER, FREE_PARTY_SLOTS}}},
};

//  The messages are the ones the party check showed before it moved onto the
//  precheck list.
const std::vector<FreePartySlotsCase> FREE_PARTY_SLOTS_CASES{
    {.name = "FiveOfSixNeedsOne", .input = {5, 1}, .expectation = {PrecheckStatus::PASSED, ""}},
    {
        .name = "FullPartyNeedsOne",
        .input = {6, 1},
        .expectation = {
            PrecheckStatus::FAILED,
            "Your party is full. Leave a party slot free so the program can check the "
            "Pokémon it catches or receives.",
        },
    },
    {.name = "FourOfSixNeedsTwo", .input = {4, 2}, .expectation = {PrecheckStatus::PASSED, ""}},
    {
        .name = "FiveOfSixNeedsTwo",
        .input = {5, 2},
        .expectation = {
            PrecheckStatus::FAILED,
            "Your party has 5 Pokémon. Leave at least 2 party slots free for the "
            "Pokémon the program catches or receives.",
        },
    },
};


}  // namespace


void add_tests_Prechecks(UnitTestDatabase& database){
    for (const RngPrechecksCase& test_case : RNG_PRECHECKS_CASES){
        database.add<Test_RngPrechecks>(test_case);
    }
    for (const FreePartySlotsCase& test_case : FREE_PARTY_SLOTS_CASES){
        database.add<Test_FreePartySlotsOutcome>(test_case);
    }
}


}  // namespace PokemonFRLG
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
