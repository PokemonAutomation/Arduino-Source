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
#include "ScaleInvariantMatrixMatch/Kernels_ScaleInvariantMatrixMatch.h"
#include "SpikeConvolution/Kernels_SpikeConvolution.h"
#include "Waterfill/Kernels_Waterfill_Tests.h"

namespace PokemonAutomation{
namespace Kernels{



void add_tests(UnitTestDatabase& database){
//  DIAG: AbsFFT::add_tests(database);
//  DIAG: AudioChannelInterleave::add_tests(database);
//  DIAG: AudioChannelMixer::add_tests(database);
//  DIAG: AudioPipeline::add_tests(database);
//  DIAG: AudioResampler::add_tests(database);
//  DIAG: AudioStreamConverter::add_tests(database);
//  DIAG: add_tests_BinaryMatrix(database);
//  DIAG: add_tests_ImageFilters(database);
//  DIAG: add_tests_ImageScaleBrightness(database);
//  DIAG: add_tests_ImageStats(database);
    ScaleInvariantMatrixMatch::add_tests(database);
//  DIAG: SpikeConvolution::add_tests(database);
//  DIAG: add_tests_Waterfill(database);
}



}
}
