/*  Pokemon SV Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Inference/PokemonSV_WhiteButtonDetector.h"
#include "Inference/Boxes/PokemonSV_BoxDetection.h"
#include "Inference/Boxes/PokemonSV_BoxEggDetector.h"
#include "Inference/Map/PokemonSV_MapDetector.h"
#include "Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "Inference/Picnics/PokemonSV_PicnicDetector.h"
#include "Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "Inference/Tera/PokemonSV_TeraCardDetector.h"
#include "Inference/Tera/PokemonSV_TeraSilhouetteReader.h"
#include "Inference/Tera/PokemonSV_TeraTypeReader.h"
#include "Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "Inference/Map/PokemonSV_FastTravelDetector.h"
#include "Inference/Map/PokemonSV_MapMenuDetector.h"
#include "Inference/Map/PokemonSV_MapPokeCenterIconDetector.h"
#include "Inference/PokemonSV_ESPEmotionDetector.h"
#include "Inference/Picnics/PokemonSV_SandwichIngredientDetector.h"
#include "Inference/Picnics/PokemonSV_SandwichPlateDetector.h"
#include "Inference/Overworld/PokemonSV_LetsGoKillDetector.h"
#include "Inference/Picnics/PokemonSV_SandwichRecipeDetector.h"
#include "Inference/Picnics/PokemonSV_SandwichHandDetector.h"
#include "Options/PokemonSV_BBQuests.h"
#include "Programs/Farming/PokemonSV_BBQLogger.h"
#include "PokemonSV_Tests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

namespace{

class Test_BBQuestsSlugs : public UnitTest{
public:
    Test_BBQuestsSlugs()
        : UnitTest("PokemonSV::BBQuests - slug round trip")
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        for (int c = 0; c < (int)BBQuests::UnableToDetect; c++){
            const BBQuests quest = (BBQuests)c;
            const std::string& slug = BBQuests_enum_to_string(quest);
            if (slug.empty()){
                return "Quest " + std::to_string(c) + " has no slug.";
            }
            if (BBQuests_string_to_enum(slug) != quest){
                return "Slug " + slug + " does not map back to quest " + std::to_string(c) + ".";
            }
        }
        if (!BBQuests_enum_to_string(BBQuests::UnableToDetect).empty()){
            return "UnableToDetect must map to the empty slug.";
        }
        if (BBQuests_string_to_enum("not-a-quest") != BBQuests::UnableToDetect){
            return "An unrecognized slug must map to UnableToDetect.";
        }
        return true;
    }
};

class BBQTestLogger : public Logger{
public:
    virtual void log(const std::string& message, Color) override{
        messages.push_back(message);
    }
    std::vector<std::string> messages;
};

class Test_BBQDiagnosticLogs : public UnitTest{
public:
    Test_BBQDiagnosticLogs(const std::string& scenario)
        : UnitTest("PokemonSV::BBQDiagnosticLogs - " + scenario), m_scenario(scenario)
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        BBQTestLogger logger;
        std::vector<std::string> expected;
        const BBQAttemptLog::Clock::time_point start{};
        if (m_scenario == "board"){
            BBQLogger(logger).board(true,
                {BBQuests::photo_swim, BBQuests::sneak_up, BBQuests::auto_10});
            BBQLogger(logger).board(false, {BBQuests::UnableToDetect});
            BBQLogger(logger).board(true, {});
            expected = {
                "BBQ event=board panel_detected=1 quests=photo-swim,sneak-up,auto-10",
                "BBQ event=board panel_detected=0 quests=unknown",
                "BBQ event=board panel_detected=1 quests=",
            };
        }else if (m_scenario == "success"){
            BBQAttemptLog attempt_log(logger, BBQuests::make_tm, 1, 2, start);
            attempt_log.route_return(start + std::chrono::milliseconds(2000));
            attempt_log.result(true, start + std::chrono::milliseconds(2500));
            expected = {
                "BBQ quest=make-tm attempt=1 limit=2 event=start",
                "BBQ quest=make-tm attempt=1 limit=2 event=route_return elapsed_ms=2000 elapsed=\"2.000 seconds\"",
                "BBQ quest=make-tm attempt=1 limit=2 event=result outcome=absent_from_board verification_ms=500 verification=\"0.500 seconds\" total_ms=2500 total=\"2.500 seconds\"",
            };
        }else if (m_scenario == "retry-exhausted"){
            BBQLogger(logger).battle_entry_timeout(BBQuests::catch_any);
            BBQAttemptLog attempt_log(logger, BBQuests::catch_any, 2, 2, start);
            attempt_log.route_return(start + std::chrono::milliseconds(3345));
            attempt_log.result(false, start + std::chrono::milliseconds(4110));
            BBQLogger(logger).retry_exhausted(BBQuests::catch_any);
            expected = {
                "BBQ quest=catch-any event=battle_entry outcome=timeout",
                "BBQ quest=catch-any attempt=2 limit=2 event=start",
                "BBQ quest=catch-any attempt=2 limit=2 event=route_return elapsed_ms=3345 elapsed=\"3.345 seconds\"",
                "BBQ quest=catch-any attempt=2 limit=2 event=result outcome=still_on_board verification_ms=765 verification=\"0.765 seconds\" total_ms=4110 total=\"4.110 seconds\"",
                "BBQ quest=catch-any event=retry_exhausted recovery=skip_current_batch",
            };
        }else if (m_scenario == "long-duration"){
            BBQAttemptLog attempt_log(logger, BBQuests::UnableToDetect, 256, 256, start);
            attempt_log.route_return(start + std::chrono::milliseconds(120000));
            attempt_log.result(false, start + std::chrono::milliseconds(120000));
            expected = {
                "BBQ quest=unknown attempt=256 limit=256 event=start",
                "BBQ quest=unknown attempt=256 limit=256 event=route_return elapsed_ms=120000 elapsed=\"2.000 minutes\"",
                "BBQ quest=unknown attempt=256 limit=256 event=result outcome=still_on_board verification_ms=0 verification=\"0.000 seconds\" total_ms=120000 total=\"2.000 minutes\"",
            };
        }else{
            return "Unknown BBQ log test scenario.";
        }
        if (logger.messages != expected){
            return "Unexpected BBQ diagnostic log sequence for " + m_scenario;
        }
        return true;
    }

private:
    const std::string m_scenario;
};

}

void add_tests(UnitTestDatabase& database){
    for (const char* scenario : {"board", "success", "retry-exhausted", "long-duration"}){
        database.add<Test_BBQDiagnosticLogs>(scenario);
    }
    database.add<Test_BBQuestsSlugs>();
    add_tests_WhiteButtonDetector(database);
    add_tests_BoxInfoDetector(database);
    add_tests_BoxEggDetector(database);
    add_tests_DialogDetector(database);
    add_tests_ESPEmotionDetector(database);
    add_tests_FastTravelDetector(database);
    add_tests_MapDetector(database);
    add_tests_MapMenuDetector(database);
    add_tests_MapPokeCenterIconDetector(database);
    add_tests_OverworldDetector(database);
    add_tests_PicnicDetector(database);
    add_tests_SandwichIngredientDetector(database);
    add_tests_SandwichPlateDetector(database);
    add_tests_SandwichRecipeDetector(database);
    add_tests_SandwichHandDetector(database);
    add_tests_TeraCardDetector(database);
    add_tests_TeraSilhouetteReader(database);
    add_tests_TeraTypeReader(database);
    add_tests_NormalBattleMenus(database);
    add_tests_LetsGoKillDetector(database);
}



}
}
}
