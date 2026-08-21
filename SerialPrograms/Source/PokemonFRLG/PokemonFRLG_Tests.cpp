/*  Pokemon FRLG Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Filesystem/Filesystem.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/StaticGlobals.h"
#include "Inference/Dialogs/PokemonFRLG_BattleDialogs.h"
#include "Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "Inference/Dialogs/PokemonFRLG_PrizeSelectDetector.h"
#include "Inference/PokemonFRLG_BattleLevelUpReader.h"
#include "Inference/PokemonFRLG_PartyLevelUpReader.h"
#include "Inference/PokemonFRLG_ShinySymbolDetector.h"
#include "Inference/PokemonFRLG_StatsReader.h"
#include "Inference/PokemonFRLG_TrainerIdReader.h"
#include "Inference/PokemonFRLG_WildEncounterReader.h"
#include "Tests/TestUtils.h"
#include "PokemonFRLG_Tests.h"

#include <fstream>
#include <sstream>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


void add_tests(UnitTestDatabase& database){
    add_tests_AdvanceWhiteDialogDetector(database);
    add_tests_SelectionDialogDetector(database);
    add_tests_ShinySymbolDetector(database);
    add_tests_PrizeSelectDetector(database);
    add_tests_AdvanceBattleDialogDetector(database);
    add_tests_BattleMenuDetector(database);
    add_tests_StatsReader(database);
    add_tests_WildEncounterReader(database);
    add_tests_TrainerIdReader(database);
    add_tests_PartyLevelUpReader(database);
    add_tests_BattleLevelUpReader(database);
}


UnitTestResult check_against_golden_file(
    const std::string& image_path,
    const std::vector<std::string>& labels,
    const std::vector<std::string>& values
){
    if (labels.size() != values.size()){
        return "Error: check_against_golden_file() called with " +
               std::to_string(labels.size()) + " labels for " +
               std::to_string(values.size()) + " values.";
    }
    for (size_t c = 0; c < values.size(); c++){
        if (values[c].empty() || values[c].find_first_of(" \t\r\n") != std::string::npos){
            return "Error: value for \"" + labels[c] + "\" is empty or contains whitespace. "
                   "The golden file cannot represent it.";
        }
    }

    Filesystem::Path file_path(image_path);
    Filesystem::Path golden_path =
            file_path.parent_path() / ("_" + file_path.stem().string() + ".txt");

    if (STATIC_GLOBALS.GENERATE_TEST_GOLDEN_FILES){
        std::ofstream output_file(golden_path.stdpath());
        if (!output_file.is_open()){
            return "Error: cannot open " + golden_path.string() + " for writing.";
        }
        for (const std::string& value : values){
            output_file << value << std::endl;
        }
        return true;
    }

    std::vector<std::string> targets;
    if (!load_slug_list(golden_path.string(), targets)){
        return "Error: cannot load golden file " + golden_path.string() + ".";
    }
    if (targets.size() != values.size()){
        return "Error: golden file " + golden_path.string() + " has " +
               std::to_string(targets.size()) + " values, expected " +
               std::to_string(values.size()) + ".";
    }
    for (size_t c = 0; c < values.size(); c++){
        TEST_RESULT_COMPONENT_EQUAL_STR(values[c], targets[c], labels[c]);
    }
    return true;
}


}
}
}