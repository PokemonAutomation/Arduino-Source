/*  FRLG Prechecks - Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  The FRLG precheck lists and verdicts: no console, no GUI.
 */

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

struct RngPrechecksCase{
    struct Input{
        size_t free_party_slots_needed;
    };
    struct Expectation{
        std::vector<ListedPrecheck> checks;
    };
    std::string name;
    Input input;
    Expectation expectation;
};

class Test_RngPrechecks : public UnitTest{
public:
    explicit Test_RngPrechecks(RngPrechecksCase test_case)
        : UnitTest("PokemonFRLG::Prechecks::RngPrechecks - " + test_case.name)
        , m_case(std::move(test_case))
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        std::vector<ListedPrecheck> listed;
        for (const std::unique_ptr<const Precheck>& check : rng_prechecks(m_case.input.free_party_slots_needed)){
            if (check->needs().empty()){
                return "Precheck " + check->name() + " must say what it needs.";
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


struct FreePartySlotsCase{
    struct Input{
        size_t party_size;
        size_t free_slots_needed;
    };
    struct Expectation{
        PrecheckStatus status;
        //  Checked only on FAILED: the message the user sees.
        std::string message;
    };
    std::string name;
    Input input;
    Expectation expectation;
};

class Test_FreePartySlotsOutcome : public UnitTest{
public:
    explicit Test_FreePartySlotsOutcome(FreePartySlotsCase test_case)
        : UnitTest("PokemonFRLG::Prechecks::FreePartySlotsOutcome - " + test_case.name)
        , m_case(std::move(test_case))
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        const PrecheckOutcome outcome = free_party_slots_outcome(
            m_case.input.party_size, m_case.input.free_slots_needed
        );
        if (outcome.status != m_case.expectation.status){
            return "Wrong verdict: \"" + outcome.message + "\"";
        }
        if (outcome.status == PrecheckStatus::FAILED && outcome.message != m_case.expectation.message){
            return "Wrong message: \"" + outcome.message + "\"";
        }
        return true;
    }

private:
    FreePartySlotsCase m_case;
};


std::vector<RngPrechecksCase> rng_prechecks_cases(){
    const ListedPrecheck black_border{"black_border", PrecheckPolicy::REQUIRED};
    const ListedPrecheck free_party_slots{"free_party_slots", PrecheckPolicy::OPTIONAL};
    return {
        {"NeedsNoSlots", {0}, {{black_border}}},
        {"NeedsOneSlot", {1}, {{black_border, free_party_slots}}},
        {"NeedsTwoSlots", {2}, {{black_border, free_party_slots}}},
    };
}

std::vector<FreePartySlotsCase> free_party_slots_cases(){
    using enum PrecheckStatus;
    return {
        {"FiveOfSixNeedsOne", {5, 1}, {PASSED, ""}},
        {
            "FullPartyNeedsOne", {6, 1},
            {
                FAILED,
                "Your party is full. Leave a party slot free so the program can check the "
                "Pokémon it catches or receives.",
            },
        },
        {"FourOfSixNeedsTwo", {4, 2}, {PASSED, ""}},
        {
            "FiveOfSixNeedsTwo", {5, 2},
            {
                FAILED,
                "Your party has 5 Pokémon. Leave at least 2 party slots free for the "
                "Pokémon the program catches or receives.",
            },
        },
    };
}


}  // namespace


void add_tests_Prechecks(UnitTestDatabase& database){
    for (RngPrechecksCase& test_case : rng_prechecks_cases()){
        database.add<Test_RngPrechecks>(std::move(test_case));
    }
    for (FreePartySlotsCase& test_case : free_party_slots_cases()){
        database.add<Test_FreePartySlotsOutcome>(std::move(test_case));
    }
}


}
}
}
