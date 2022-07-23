/*  PokemonLA Tests
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *  
 *  
 */


#ifndef PokemonAutomation_Tests_PokemonSwSh_Tests_H
#define PokemonAutomation_Tests_PokemonSwSh_Tests_H

class QImage;

#include <vector>
#include <string>

namespace PokemonAutomation{

int test_pokemonSwSh_YCommMenuDetector(const QImage& image, bool target);

int test_pokemonSwSh_MaxLair_BattleMenuDetector(const QImage& image, bool target);


}

#endif
