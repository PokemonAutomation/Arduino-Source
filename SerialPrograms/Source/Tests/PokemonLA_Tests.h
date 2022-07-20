/*  PokemonLA Tests
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *  
 *  
 */


#ifndef PokemonAutomation_Tests_PokemonLA_Tests_H
#define PokemonAutomation_Tests_PokemonLA_Tests_H

class QImage;

#include <vector>
#include <string>

#include "CommonFramework/AudioPipeline/AudioFeed.h"

namespace PokemonAutomation{

int test_pokemonLA_BattleMenuDetector(const QImage& image, bool target);

int test_pokemonLA_BattlePokemonSwitchDetector(const QImage& image, bool target);

int test_pokemonLA_TransparentDialogueDetector(const QImage& image, bool target);

int test_pokemonLA_EventDialogDetector(const QImage& image, bool target);

int test_pokemonLA_DialogueYellowArrowDetector(const QImage& image, bool target);

int test_pokemonLA_BlackOutDetector(const QImage& image, bool target);

int test_pokemonLA_MMOQuestionMarkDetector(const QImage& image,  const std::vector<std::string>& keywords);

int test_pokemonLA_StatusInfoScreenDetector(const QImage& image, const std::vector<std::string>& keywords);

int test_pokemonLA_MapMarkerLocator(const QImage& image, float target_angle, float threshold);

int test_pokemonLA_MapZoomLevelReader(const QImage& image, int target);

int test_pokemonLA_MapMissionTabReader(const QImage& image, bool target);

void test_pokemonLA_BerryTreeDetector(const QImage& image);

int test_pokemonLA_shinySoundDetector(const std::vector<AudioSpectrum>& spectrums, bool target);

int test_pokemonLA_MMOSpriteMatcher(const std::string& filepath);

}

#endif
