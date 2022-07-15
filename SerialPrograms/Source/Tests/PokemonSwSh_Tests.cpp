/*  PokemonLA Tests
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "Common/Compiler.h"
#include "PokemonSwSh_Tests.h"
#include "TestUtils.h"

#include "PokemonSwSh/Inference/PokemonSwSh_YCommDetector.h"

#include <QFileInfo>
#include <QImage>
#include <QDir>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>
using std::cout;
using std::cerr;
using std::endl;

namespace PokemonAutomation{

using namespace NintendoSwitch::PokemonSwSh;

int test_pokemonSwSh_YCommMenuDetector(const QImage& image, bool target){
    YCommMenuDetector detector(true);

    bool result = detector.process_frame(image, current_time());
    TEST_RESULT_EQUAL(result, target);
    return 0;
}


}
