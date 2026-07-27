/*  Pokemon LA Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Inference/PokemonLA_DialogDetector.h"
#include "Inference/PokemonLA_BlackOutDetector.h"
#include "Inference/PokemonLA_StatusInfoScreenDetector.h"
#include "Inference/PokemonLA_WildPokemonFocusDetector.h"
#include "Inference/PokemonLA_BerryTreeDetector.h"
#include "Inference/Objects/PokemonLA_DialogYellowArrowDetector.h"
#include "Inference/Objects/PokemonLA_FlagTracker.h"
#include "Inference/Objects/PokemonLA_MMOQuestionMarkDetector.h"
#include "Inference/Objects/PokemonLA_BattleSpriteArrowDetector.h"
#include "Inference/Map/PokemonLA_MapZoomLevelReader.h"
#include "Inference/Map/PokemonLA_PokemonMapSpriteReader.h"
#include "Inference/Map/PokemonLA_MapMarkerLocator.h"
#include "Inference/Map/PokemonLA_MapMissionTabReader.h"
#include "Inference/Map/PokemonLA_MapWeatherAndTimeReader.h"
#include "Inference/Battles/PokemonLA_TransparentDialogueDetector.h"
#include "Inference/Battles/PokemonLA_BattleMenuDetector.h"
#include "Inference/Battles/PokemonLA_BattlePokemonSwitchDetector.h"
#include "Inference/Battles/PokemonLA_BattleSpriteWatcher.h"
#include "Inference/Battles/PokemonLA_BattleStartDetector.h"
#include "Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "Programs/PokemonLA_GameSave.h"
#include "PokemonLA_Tests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



void add_tests(UnitTestDatabase& database){
    add_tests_DialogDetector(database);
    add_tests_BlackOutDetector(database);
    add_tests_StatusInfoScreenDetector(database);
    add_tests_WildPokemonFocusDetector(database);
    add_tests_BerryTreeDetector(database);

    add_tests_DialogYellowArrowDetector(database);
    add_tests_FlagTracker(database);
    add_tests_MMOQuestionMarkDetector(database);
    add_tests_BattleSpriteArrowDetector(database);

    add_tests_MapZoomLevelReader(database);
    add_tests_PokemonMapSpriteReader(database);
    add_tests_MapMarkerLocator(database);
    add_tests_MapMissionTabReader(database);
    add_tests_MapWeatherAndTimeReader(database);

    add_tests_TransparentDialogueDetector(database);
    add_tests_BattleMenuDetector(database);
    add_tests_BattlePokemonSwitchDetector(database);
    add_tests_BattleSpriteWatcher(database);
    add_tests_BattleStartDetector(database);

    add_tests_ShinySoundDetector(database);

    add_tests_GameSave(database);
}



}
}
}
