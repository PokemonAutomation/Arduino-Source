/*  Max Lair State Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <format>
#include <utility>
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "VideoGames/PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_State.h"
#include "PokemonSwSh_MaxLair_State_Tests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{
namespace{

struct BossStateTestCaseInput{
    std::string name;
    uint8_t wins;
    std::set<std::string> opponent;
    std::string boss;
    uint8_t final_wins;     //  wins on the caught screen
};

struct BossStateTestCaseExpectation{
    bool accepted;
    std::string boss;
    std::string boss_if_caught;
};

struct BossStateTestCase{
    BossStateTestCaseInput input;
    BossStateTestCaseExpectation expectation;
};

class Test_MaxLairBossState : public UnitTest{
public:
    explicit Test_MaxLairBossState(BossStateTestCase scenario)
        : UnitTest("PokemonSwSh::MaxLairBossState - " + scenario.input.name)
        , m_scenario(std::move(scenario))
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        const BossStateTestCaseInput& input = m_scenario.input;
        const BossStateTestCaseExpectation& expected = m_scenario.expectation;
        GlobalState state;
        state.wins = input.wins;
        state.opponent = input.opponent;
        state.boss = input.boss;
        const bool accepted = state.accept_opponent();
        if (accepted != expected.accepted){
            return std::format("Expected opponent accepted={}, got {}", expected.accepted, accepted);
        }
        if (state.boss != expected.boss){
            return std::format("Expected boss '{}', got '{}'", expected.boss, state.boss);
        }
        state.wins = input.final_wins;
        const std::string caught = state.boss_if_caught();
        if (caught != expected.boss_if_caught){
            return std::format("Expected boss_if_caught '{}', got '{}'", expected.boss_if_caught, caught);
        }
        return true;
    }

private:
    BossStateTestCase m_scenario;
};

const BossStateTestCase CASES[] = {
    {
        .input = {.name = "rental before three wins", .wins = 2, .opponent = {"mimikyu-disguised"}, .boss = "", .final_wins = 3},
        .expectation = {.accepted = true, .boss = "", .boss_if_caught = ""},
    },
    {
        .input = {.name = "Mimikyu at boss stage must retry", .wins = 3, .opponent = {"mimikyu-disguised"}, .boss = "", .final_wins = 3},
        .expectation = {.accepted = false, .boss = "", .boss_if_caught = ""},
    },
    {
        .input = {.name = "Kyurem after Mimikyu misidentification", .wins = 3, .opponent = {"kyurem"}, .boss = "mimikyu-disguised", .final_wins = 3},
        .expectation = {.accepted = true, .boss = "kyurem", .boss_if_caught = ""},
    },
    {
        .input = {.name = "identified boss lost", .wins = 3, .opponent = {"kyurem"}, .boss = "kyurem", .final_wins = 3},
        .expectation = {.accepted = true, .boss = "kyurem", .boss_if_caught = ""},
    },
    {
        .input = {.name = "identified boss caught", .wins = 3, .opponent = {"kyurem"}, .boss = "", .final_wins = 4},
        .expectation = {.accepted = true, .boss = "kyurem", .boss_if_caught = "kyurem"},
    },
    {
        .input = {.name = "boss before three wins must retry", .wins = 2, .opponent = {"kyurem"}, .boss = "", .final_wins = 2},
        .expectation = {.accepted = false, .boss = "", .boss_if_caught = ""},
    },
    {
        .input = {.name = "empty boss-stage read must retry", .wins = 3, .opponent = {}, .boss = "", .final_wins = 3},
        .expectation = {.accepted = false, .boss = "", .boss_if_caught = ""},
    },
    {
        .input = {.name = "ambiguous boss-stage read must retry", .wins = 3, .opponent = {"kyurem", "mimikyu-disguised"}, .boss = "", .final_wins = 3},
        .expectation = {.accepted = false, .boss = "", .boss_if_caught = ""},
    },
    {
        .input = {.name = "rejected read preserves known boss", .wins = 3, .opponent = {"mimikyu-disguised"}, .boss = "kyurem", .final_wins = 3},
        .expectation = {.accepted = false, .boss = "kyurem", .boss_if_caught = ""},
    },
    {
        .input = {.name = "fourth win cannot legitimize rental boss", .wins = 3, .opponent = {"mimikyu-disguised"}, .boss = "mimikyu-disguised", .final_wins = 4},
        .expectation = {.accepted = false, .boss = "mimikyu-disguised", .boss_if_caught = ""},
    },
};

}

void add_tests_MaxLairBossState(UnitTestDatabase& database){
    for (const BossStateTestCase& scenario : CASES){
        database.add<Test_MaxLairBossState>(scenario);
    }
}

}
}
}
}
