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
        .input = {.name = "Mimikyu at boss stage is not the boss", .wins = 3, .opponent = {"mimikyu-disguised"}, .boss = "", .final_wins = 3},
        .expectation = {.accepted = true, .boss = "", .boss_if_caught = ""},
    },
    {
        .input = {.name = "Kyurem after a rejected Mimikyu read", .wins = 3, .opponent = {"kyurem"}, .boss = "", .final_wins = 4},
        .expectation = {.accepted = true, .boss = "kyurem", .boss_if_caught = "kyurem"},
    },
    //  A wrong boss can no longer be recorded (see the cases above); one that
    //  is already known is kept.
    {
        .input = {.name = "Kyurem after Mimikyu misidentification", .wins = 3, .opponent = {"kyurem"}, .boss = "mimikyu-disguised", .final_wins = 3},
        .expectation = {.accepted = true, .boss = "mimikyu-disguised", .boss_if_caught = ""},
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
        .input = {.name = "empty boss-stage read", .wins = 3, .opponent = {}, .boss = "", .final_wins = 3},
        .expectation = {.accepted = true, .boss = "", .boss_if_caught = ""},
    },
    {
        .input = {.name = "ambiguous boss-stage read is not the boss", .wins = 3, .opponent = {"kyurem", "mimikyu-disguised"}, .boss = "", .final_wins = 3},
        .expectation = {.accepted = true, .boss = "", .boss_if_caught = ""},
    },
    {
        .input = {.name = "rental read after known boss", .wins = 3, .opponent = {"mimikyu-disguised"}, .boss = "kyurem", .final_wins = 3},
        .expectation = {.accepted = true, .boss = "kyurem", .boss_if_caught = ""},
    },
};

struct FitsStageTestCaseInput{
    std::string name;
    uint8_t wins;
    std::set<std::string> opponent;
    std::string boss;
};

struct FitsStageTestCaseExpectation{
    bool fits;
};

struct FitsStageTestCase{
    FitsStageTestCaseInput input;
    FitsStageTestCaseExpectation expectation;
};

class Test_MaxLairFitsStage : public UnitTest{
public:
    explicit Test_MaxLairFitsStage(FitsStageTestCase scenario)
        : UnitTest(std::format("PokemonSwSh::MaxLairFitsStage - {}", scenario.input.name))
        , m_scenario(std::move(scenario))
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        const FitsStageTestCaseInput& input = m_scenario.input;
        const FitsStageTestCaseExpectation& expected = m_scenario.expectation;
        GlobalState state;
        state.wins = input.wins;
        state.opponent = input.opponent;
        state.boss = input.boss;
        const bool fits = state.opponent_fits_stage();
        if (fits != expected.fits){
            return std::format("Expected fits={}, got {}", expected.fits, fits);
        }
        return true;
    }

private:
    FitsStageTestCase m_scenario;
};

const FitsStageTestCase FITS_STAGE_CASES[] = {
    {
        .input = {.name = "rental before three wins", .wins = 2, .opponent = {"mimikyu-disguised"}, .boss = ""},
        .expectation = {.fits = true},
    },
    {
        .input = {.name = "boss before three wins", .wins = 2, .opponent = {"kyurem"}, .boss = ""},
        .expectation = {.fits = false},
    },
    {
        .input = {.name = "rental at boss stage with unknown boss", .wins = 3, .opponent = {"mimikyu-disguised"}, .boss = ""},
        .expectation = {.fits = false},
    },
    {
        .input = {.name = "empty read at boss stage with unknown boss", .wins = 3, .opponent = {}, .boss = ""},
        .expectation = {.fits = false},
    },
    {
        .input = {.name = "ambiguous read at boss stage with unknown boss", .wins = 3, .opponent = {"kyurem", "mimikyu-disguised"}, .boss = ""},
        .expectation = {.fits = false},
    },
    {
        .input = {.name = "boss at boss stage", .wins = 3, .opponent = {"kyurem"}, .boss = ""},
        .expectation = {.fits = true},
    },
    {
        .input = {.name = "rental at boss stage with known boss", .wins = 3, .opponent = {"mimikyu-disguised"}, .boss = "kyurem"},
        .expectation = {.fits = false},
    },
    {
        .input = {.name = "known boss at boss stage", .wins = 3, .opponent = {"kyurem"}, .boss = "kyurem"},
        .expectation = {.fits = true},
    },
    {
        .input = {.name = "other boss at boss stage with known boss", .wins = 3, .opponent = {"zekrom"}, .boss = "kyurem"},
        .expectation = {.fits = false},
    },
};
}

void add_tests_MaxLairBossState(UnitTestDatabase& database){
    for (const BossStateTestCase& scenario : CASES){
        database.add<Test_MaxLairBossState>(scenario);
    }
    for (const FitsStageTestCase& scenario : FITS_STAGE_CASES){
        database.add<Test_MaxLairFitsStage>(scenario);
    }
}

}
}
}
}
