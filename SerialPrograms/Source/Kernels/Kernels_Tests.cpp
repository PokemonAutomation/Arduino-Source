/*  Kernels Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels_Tests.h"
#include "AbsFFT/Kernels_AbsFFT.h"
#include "AudioChannelInterleave/Kernels_AudioChannelInterleave.h"
#include "AudioChannelMixer/Kernels_AudioChannelMixer.h"
#include "AudioPipeline/Kernels_AudioPipeline.h"
#include "AudioResampler/Kernels_AudioResampler_Tests.h"
#include "AudioStreamConverter/Kernels_AudioStreamConverter.h"
#include "BinaryMatrix/Kernels_BinaryMatrix_Tests.h"
#include "ImageFilters/Kernels_ImageFilter_Tests.h"
#include "ImageScaleBrightness/Kernels_ImageScaleBrightness_Tests.h"
#include "ImageStats/Kernels_ImageStats_Tests.h"
#include "SpikeConvolution/Kernels_SpikeConvolution.h"
#include "Waterfill/Kernels_Waterfill_Tests.h"

namespace PokemonAutomation{
namespace Kernels{



void add_tests(UnitTestDatabase& database){
    AbsFFT::add_tests(database);
    AudioChannelInterleave::add_tests(database);
    AudioChannelMixer::add_tests(database);
    AudioPipeline::add_tests(database);
    AudioResampler::add_tests(database);
    AudioStreamConverter::add_tests(database);
    add_tests_BinaryMatrix(database);
    add_tests_ImageFilters(database);
    add_tests_ImageScaleBrightness(database);
    add_tests_ImageStats(database);
    SpikeConvolution::add_tests(database);
    add_tests_Waterfill(database);
}



}
}
