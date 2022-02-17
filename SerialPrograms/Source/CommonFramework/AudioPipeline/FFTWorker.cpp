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
#include "Kernels/AbsFFT/Kernels_AbsFFT.h"
#include "Common/Cpp/AlignedVector.tpp"
#include "CommonFramework/Logging/Logger.h"
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
    , m_fftInputBuffer(m_fftLength)
    , m_fftOutputBuffer(m_fftLength / 2)
{
    m_outputVector.resize((int)(m_fftLength / 2));
}

FFTWorker::~FFTWorker(){}

void FFTWorker::computeFFT(const QVector<float>& fftInput){
    // float sum = 0.0;
    // for(auto v : rawAudioSamples) sum += v;
    // std::cout << "FFTWorker::computeFFT, receive fft input, sum " << sum << std::endl;;

    // We need to copy data from one buffer to another because m_fftInputBuffer is 
    // memory aligned. This is required for the FFT function.
    memcpy(m_fftInputBuffer.data(), fftInput.data(), sizeof(float) * m_fftInputBuffer.size());

    // raw audio samples are in range [-1.0, 1.0]. Need to convert it to [0.0, 1.0]
    // for(size_t i = 0; i < m_fftInputBuffer.size(); i++){
    //     m_fftInputBuffer[i] = (rawAudioSamples[i] + 1.0) / 2.0;
    // }

//    auto startTime = std::chrono::system_clock::now();

#ifdef USE_FFTREAL
    FFTRealWrapper fft;
    std::vector<float> FFTRealOutputBuffer(m_fftInputBuffer.size())
    fft.calculateFFT(FFTRealOutputBuffer.data(), m_fftInputBuffer.data());

    num_fft_samples = m_fftInputBuffer.size();
    for(int i = 2; i <= num_fft_samples/2; i++){
        float real = m_fftOutputBuffer[i];
        float imag = 0.0;
        if (i>0 && i<num_fft_samples/2) {
            imag = m_fftOutputBuffer[num_fft_samples/2 + i];
        }
        float mag = std::sqrt(real * real + imag * imag);
        // std::cout << mag << " ";
        m_fftOutputBuffer[i-2] = mag;
    }
#else
    Kernels::AbsFFT::fft_abs(m_fftLengthPowerOfTwo, m_fftOutputBuffer.data(), m_fftInputBuffer.data());
#endif
    // The constant part of the frequency is the sum of all audio sample values.
    // It may be negative. Use fabs() to get its magnitude.
    m_fftOutputBuffer[0] = std::fabs(m_fftOutputBuffer[0]);

//    auto endTime = std::chrono::system_clock::now();
//    std::chrono::microseconds dur = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    // std::cout << "T" << QThread::currentThread() << " FFTWorker::computeFFT() finished, time " dur.count() <<
    //     " ms" << std::endl;

    memcpy(m_outputVector.data(), m_fftOutputBuffer.data(), sizeof(float) * m_outputVector.size());
    emit FFTFinished(m_outputVector);
}



}
