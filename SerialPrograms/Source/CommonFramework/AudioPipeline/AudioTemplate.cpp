/*  Audio Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "AudioConstants.h"
#include "AudioTemplate.h"
#include "AudioFileLoader.h"
#include "AudioFormatUtils.h"
#include "FFTWorker.h"

#include <iostream>


namespace PokemonAutomation{


AudioTemplate::AudioTemplate(std::vector<float>&& spectrogram, size_t numWindows)
    :m_numWindows(numWindows)
    ,m_numFrequencies(spectrogram.size()/numWindows)
    ,m_spectrogram(std::move(spectrogram)) {}


AudioTemplate loadAudioTemplate(const QString& filename){
    QAudioFormat outputAudioFormat;
    outputAudioFormat.setChannelCount(1);
#if QT_VERSION_MAJOR == 5
    outputAudioFormat.setCodec("audio/pcm");
#endif
    outputAudioFormat.setSampleRate(48000);
    setSampleFormatToFloat(outputAudioFormat);
    
    AudioFileLoader loader(nullptr, filename, outputAudioFormat);
    const auto ret = loader.loadFullAudio();
    const float* data = reinterpret_cast<const float*>(std::get<0>(ret));
    size_t numSamples = std::get<1>(ret) / sizeof(float);
    
    if (data == nullptr){
        return AudioTemplate();
    }

    FFTWorker fft(FFT_LENGTH_POWER_OF_TWO);
    const size_t numFrequencies = fft.outputSize();

    size_t numWindows = 0;
    std::vector<float> spectrogram;

    // If sample count < FFT input requirement, we pad zeros in the end to do one FFT.
    // Otherwise, we don't pad zeros and compute FFT as much as possible using fixed
    // window step.
    if (numSamples < NUM_FFT_SAMPLES){
        numWindows = 1;
        spectrogram.resize(numFrequencies);

        std::vector<float> paddedBuffer(NUM_FFT_SAMPLES);
        memcpy(paddedBuffer.data(), data, sizeof(float) * numSamples);
        const float* fftOutput = fft.fftKernel(paddedBuffer.data());
        memcpy(spectrogram.data(), fftOutput, sizeof(float) * numFrequencies);
    } else{
        numWindows = (numSamples - NUM_FFT_SAMPLES) / FFT_SLIDING_WINDOW_STEP + 1;
        spectrogram.resize(numWindows * numFrequencies);

        for(size_t i = 0, start = 0; start+NUM_FFT_SAMPLES <= numSamples; i++, start += FFT_SLIDING_WINDOW_STEP){
            assert(i < numWindows);
            const float* fftOutput = fft.fftKernel(data + i);
            memcpy(spectrogram.data() + i*numFrequencies, fftOutput, sizeof(float) * numFrequencies);
        }
    }

    std::cout << "Built audio template with " << numWindows << " windows and " << numFrequencies << 
        " frequenices from " << filename.toStdString() << std::endl;

    return AudioTemplate(std::move(spectrogram), numWindows);
}


















}