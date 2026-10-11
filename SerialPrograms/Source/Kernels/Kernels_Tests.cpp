/*  Kernels Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels_Tests.h"
#include "AudioChannelInterleave/Kernels_AudioChannelInterleave.h"
#include "AudioChannelMixer/Kernels_AudioChannelMixer.h"
#include "AudioStreamConverter/Kernels_AudioStreamConverter.h"
#include "BinaryMatrix/Kernels_BinaryMatrix_Tests.h"
#include "ImageFilters/Kernels_ImageFilter_Tests.h"
#include "ImageScaleBrightness/Kernels_ImageScaleBrightness_Tests.h"
#include "Waterfill/Kernels_Waterfill_Tests.h"

namespace PokemonAutomation{
namespace Kernels{



void add_tests(UnitTestDatabase& database){
    AudioChannelInterleave::add_tests(database);
    AudioChannelMixer::add_tests(database);
    AudioStreamConverter::add_tests(database);
    add_tests_BinaryMatrix(database);
    add_tests_ImageFilters(database);
    add_tests_ImageScaleBrightness(database);
    add_tests_Waterfill(database);
}



}
}
