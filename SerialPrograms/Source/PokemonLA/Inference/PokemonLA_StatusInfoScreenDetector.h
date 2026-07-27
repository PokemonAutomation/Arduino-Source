/*  Status Info Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Detect shiny and alpha on the Status Info Screen in a battle.
 */

#ifndef PokemonAutomation_PokemonLA_StatusInfoScreenDetector_H
#define PokemonAutomation_PokemonLA_StatusInfoScreenDetector_H

#include "Common/Cpp/TestRunners/UnitTest.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "PokemonLA/Resources/PokemonLA_PokemonInfo.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


PokemonDetails read_status_info(
    Logger& logger, VideoOverlay& overlay,
    const ImageViewRGB32& frame,
    Language language
);



std::string read_pokemon_info_from_words(const std::vector<std::string>& keywords, Language& language, PokemonDetails& details);
std::string test_pokemon_details(const PokemonDetails& details, const PokemonDetails& target);

void add_tests_StatusInfoScreenDetector(UnitTestDatabase& database);



}
}
}
#endif
