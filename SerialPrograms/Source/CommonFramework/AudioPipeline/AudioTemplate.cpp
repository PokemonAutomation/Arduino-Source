/*  Audio Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/AlignedVector.tpp"
#include "Kernels/AbsFFT/Kernels_AbsFFT.h"
#include "AudioConstants.h"
#include "AudioTemplate.h"
#include "AudioFileLoader.h"
#include "AudioFormatUtils.h"
#include "FFTWorker.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


AudioTemplate::~AudioTemplate(){}
AudioTemplate::AudioTemplate(){}
AudioTemplate::AudioTemplate(AlignedVector<float>&& spectrogram, size_t numWindows)
    : m_numWindows(numWindows)
    , m_numFrequencies(spectrogram.size()/numWindows)
    , m_spectrogram(std::move(spectrogram))
{
    cout << "numWindows = " << numWindows << endl;
    cout << "m_numFrequencies = " << m_numFrequencies << endl;  //  REMOVE
}


AudioTemplate loadAudioTemplate(const QString& filename, size_t sampleRate){
    QAudioFormat outputAudioFormat;
    outputAudioFormat.setChannelCount(1);
#if QT_VERSION_MAJOR == 5
    outputAudioFormat.setCodec("audio/pcm");
#endif
    outputAudioFormat.setSampleRate((int)sampleRate);
    setSampleFormatToFloat(outputAudioFormat);
    
    AudioFileLoader loader(nullptr, filename, outputAudioFormat);
    const auto ret = loader.loadFullAudio();
    const float* data = reinterpret_cast<const float*>(std::get<0>(ret));
    size_t numSamples = std::get<1>(ret) / sizeof(float);
    
    if (data == nullptr){
        return AudioTemplate();
    }

    size_t numFrequencies = NUM_FFT_SAMPLES / 2;
    AlignedVector<float> input_buffer(NUM_FFT_SAMPLES);
    AlignedVector<float> output_buffer(numFrequencies);

    size_t numWindows = 0;
    AlignedVector<float> spectrogram;

    cout << "numSamples = " << numSamples << endl;  //  REMOVE

    // If sample count < FFT input requirement, we pad zeros in the end to do one FFT.
    // Otherwise, we don't pad zeros and compute FFT as much as possible using fixed
    // window step.
    if (numSamples < NUM_FFT_SAMPLES){
        numWindows = 1;
        spectrogram = AlignedVector<float>(numFrequencies);

        memset(input_buffer.data(), 0, sizeof(float) * NUM_FFT_SAMPLES);
        memcpy(input_buffer.data(), data, sizeof(float) * numSamples);
        Kernels::AbsFFT::fft_abs(FFT_LENGTH_POWER_OF_TWO, output_buffer.data(), input_buffer.data());
        memcpy(spectrogram.data(), output_buffer.data(), sizeof(float) * numFrequencies);
    }else{
        numWindows = (numSamples - NUM_FFT_SAMPLES) / FFT_SLIDING_WINDOW_STEP + 1;
        spectrogram = AlignedVector<float>(numWindows * numFrequencies);

        for (size_t i = 0, start = 0; start+NUM_FFT_SAMPLES <= numSamples; i++, start += FFT_SLIDING_WINDOW_STEP){
            assert(i < numWindows);
            memcpy(input_buffer.data(), data + start, sizeof(float) * NUM_FFT_SAMPLES);
            Kernels::AbsFFT::fft_abs(FFT_LENGTH_POWER_OF_TWO, output_buffer.data(), input_buffer.data());
            memcpy(spectrogram.data() + i*numFrequencies, output_buffer.data(), sizeof(float) * numFrequencies);
        }
    }

    std::cout << "Built audio template with sample rate " << sampleRate << ", " << numWindows << " windows and " << numFrequencies << 
        " frequenices from " << filename.toStdString() << std::endl;

    return AudioTemplate(std::move(spectrogram), numWindows);
}


















}
