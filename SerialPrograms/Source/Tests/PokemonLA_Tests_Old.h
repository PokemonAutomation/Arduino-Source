/*  PokemonLA Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *  
 *  
 */


#ifndef PokemonAutomation_Tests_PokemonLA_Tests_H
#define PokemonAutomation_Tests_PokemonLA_Tests_H

#include <vector>
#include <string>

#include "CommonFramework/AudioPipeline/AudioFeed.h"

namespace PokemonAutomation{

class ImageViewRGB32;


int test_pokemonLA_BattleMenuDetector(const ImageViewRGB32& image, bool target);

int test_pokemonLA_BattlePokemonSwitchDetector(const ImageViewRGB32& image, bool target);

int test_pokemonLA_TransparentDialogueDetector(const ImageViewRGB32& image, bool target);

int test_pokemonLA_EventDialogDetector(const ImageViewRGB32& image, bool target);

int test_pokemonLA_DialogueYellowArrowDetector(const ImageViewRGB32& image, bool target);

int test_pokemonLA_BlackOutDetector(const ImageViewRGB32& image, bool target);

int test_pokemonLA_BattleStartDetector(const ImageViewRGB32& image, bool target);

int test_pokemonLA_MMOQuestionMarkDetector(const ImageViewRGB32& image,  const std::vector<std::string>& keywords);

int test_pokemonLA_StatusInfoScreenDetector(const ImageViewRGB32& image, const std::vector<std::string>& keywords);

int test_pokemonLA_WildPokemonFocusDetector(const ImageViewRGB32& image, const std::vector<std::string>& keywords);

int test_pokemonLA_BattleSpriteWatcher(const ImageViewRGB32& image, const std::vector<std::string>& keywords);

int test_pokemonLA_SaveScreenDetector(const ImageViewRGB32& image, const std::vector<std::string>& keywords);

int test_pokemonLA_MapMarkerLocator(const ImageViewRGB32& image, float target_angle, float threshold);

int test_pokemonLA_MapZoomLevelReader(const ImageViewRGB32& image, int target);

int test_pokemonLA_BattleSpriteArrowDetector(const ImageViewRGB32& image, int target);

int test_pokemonLA_MapMissionTabReader(const ImageViewRGB32& image, bool target);

int test_pokemonLA_BerryTreeDetector(const ImageViewRGB32& image);

int test_pokemonLA_shinySoundDetector(const std::vector<AudioSpectrum>& spectrums, bool target);

int test_pokemonLA_MMOSpriteMatcher(const std::string& filepath);

int test_pokemonLA_MapWeatherAndTimeReader(const ImageViewRGB32& image, const std::vector<std::string>& keywords);

int test_pokemonLA_FlagTracker_performance(const ImageViewRGB32& image, int num_iterations);

}

#endif
