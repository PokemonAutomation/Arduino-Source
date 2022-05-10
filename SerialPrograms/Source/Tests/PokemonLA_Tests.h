/*  PokemonLA Tests
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *  
 *  
 */


#ifndef PokemonAutomation_Tests_PokemonLA_Tests_H
#define PokemonAutomation_Tests_PokemonLA_Tests_H

class QImage;


namespace PokemonAutomation{

int test_pokemonLA_BattleMenuDetector(const QImage& image, bool target);

int test_pokemonLA_BattlePokemonSwitchDetector(const QImage& image, bool target);

int test_pokemonLA_DialogueEllipseDetector(const QImage& image, bool target);

void test_pokemonLA_BerryTreeDetector(const QImage& image);

}

#endif
