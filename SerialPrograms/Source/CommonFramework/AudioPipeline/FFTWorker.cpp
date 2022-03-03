/*  Audio Thread Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cfloat>
#include <chrono>
#include <memory>
#include <cmath>
#include <QThread>
#include "Common/Cpp/AlignedVector.tpp"
#include "Kernels/AbsFFT/Kernels_AbsFFT.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "AudioDisplayWidget.h"
#include "FFTWorker.h"


// #define USE_FFTREAL

#ifdef USE_FFTREAL
#include <fftreal_wrapper.h>
#endif

// #define DEBUG_AUDIO_IO

namespace PokemonAutomation{


FFTWorker::FFTWorker(int fftLengthPowerOfTwo)
    : m_fftLengthPowerOfTwo(fftLengthPowerOfTwo)
    , m_fftLength((size_t)1 << m_fftLengthPowerOfTwo)
{}

FFTWorker::~FFTWorker(){}


void FFTWorker::computeFFT(std::shared_ptr<AlignedVector<float>> fftInput){
    std::shared_ptr<AlignedVector<float>> out = std::make_unique<AlignedVector<float>>(m_fftLength / 2);

    Kernels::AbsFFT::fft_abs(m_fftLengthPowerOfTwo, out->data(), fftInput->data());

    emit FFTFinished(std::move(out));
}



}
