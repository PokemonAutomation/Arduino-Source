/*  Audio Template
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <sstream>
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "CommonFramework/Logging/Logger.h"
#include "Kernels/Kernels_Alignment.h"
#include "Kernels/AbsFFT/Kernels_AbsFFT.h"
#include "AudioConstants.h"
#include "AudioTemplate.h"
#include "Tools/AudioFormatUtils.h"
#include "IO/AudioFileLoader.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


AudioTemplate::~AudioTemplate(){}
AudioTemplate::AudioTemplate(AudioTemplate&& x) = default;
AudioTemplate& AudioTemplate::operator=(AudioTemplate&& x) = default;
AudioTemplate::AudioTemplate(const AudioTemplate&) = default;
AudioTemplate& AudioTemplate::operator=(const AudioTemplate&) = default;

AudioTemplate::AudioTemplate(){}
AudioTemplate::AudioTemplate(size_t frequencies, size_t windows)
    : m_numWindows(windows)
    , m_numFrequencies(frequencies)
    , m_bytes_per_spectrum(Kernels::align_int_up<PA_ALIGNMENT>(frequencies * sizeof(float)))
    , m_spectrogram(windows * (m_bytes_per_spectrum / sizeof(float)))
{}


AudioTemplate loadAudioTemplate(const std::string& filename, size_t sample_rate){
    QAudioFormat outputAudioFormat;
    outputAudioFormat.setChannelCount(1);
#if QT_VERSION_MAJOR == 5
    outputAudioFormat.setCodec("audio/pcm");
#endif
    outputAudioFormat.setSampleRate((int)sample_rate);
    setSampleFormatToFloat(outputAudioFormat);
    
    AudioFileLoader loader(nullptr, filename, outputAudioFormat);
    const auto ret = loader.loadFullAudio();
    const float* data = reinterpret_cast<const float*>(std::get<0>(ret));
    size_t numSamples = std::get<1>(ret) / sizeof(float);
//    cout << "numSamples = " << numSamples << endl;
    
    if (data == nullptr){
        return AudioTemplate();
    }

    size_t numFrequencies = NUM_FFT_SAMPLES / 2;
    AlignedVector<float> input_buffer(NUM_FFT_SAMPLES);
    AlignedVector<float> output_buffer(numFrequencies);

    size_t numWindows = 0;
    AudioTemplate audio_template;

    // If sample count < FFT input requirement, we pad zeros in the end to do one FFT.
    // Otherwise, we don't pad zeros and compute FFT as much as possible using fixed
    // window step.
    if (numSamples < NUM_FFT_SAMPLES){
        numWindows = 1;
        audio_template = AudioTemplate(numFrequencies, 1);

        memset(input_buffer.data(), 0, sizeof(float) * NUM_FFT_SAMPLES);
        memcpy(input_buffer.data(), data, sizeof(float) * numSamples);
        Kernels::AbsFFT::fft_abs(FFT_LENGTH_POWER_OF_TWO, output_buffer.data(), input_buffer.data());
        memcpy(audio_template.getWindow(0), output_buffer.data(), sizeof(float) * numFrequencies);
    }else{
        numWindows = (numSamples - NUM_FFT_SAMPLES) / FFT_SLIDING_WINDOW_STEP + 1;
        audio_template = AudioTemplate(numFrequencies, numWindows);

        for (size_t i = 0, start = 0; start+NUM_FFT_SAMPLES <= numSamples; i++, start += FFT_SLIDING_WINDOW_STEP){
            assert(i < numWindows);
            memcpy(input_buffer.data(), data + start, sizeof(float) * NUM_FFT_SAMPLES);
            Kernels::AbsFFT::fft_abs(FFT_LENGTH_POWER_OF_TWO, output_buffer.data(), input_buffer.data());
            memcpy(audio_template.getWindow(i), output_buffer.data(), sizeof(float) * numFrequencies);
        }
    }


    std::stringstream ss;
    ss << "Built audio template with sample rate " << sample_rate << ", " << numWindows << " windows and " << numFrequencies <<
        " frequencies from " << filename;
    global_logger_tagged().log(ss.str());

    return audio_template;
}


















}
