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

int test_pokemonLA_DialogueYellowArrowDetector(const QImage& image, bool target);

int test_pokemonLA_StatusInfoScreenDetector(const QImage& image, const std::vector<std::string>& keywords);

void test_pokemonLA_BerryTreeDetector(const QImage& image);

int test_pokemonLA_shinySoundDetector(const std::vector<AudioSpectrum>& spectrums, bool target);

}

#endif
