/*  Audio Template
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <sstream>
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Common/Qt/GlobalThreadPoolsQt.h"
#include "Kernels/Kernels_Alignment.h"
#include "Kernels/KaiserWindow/Kernels_KaiserWindow.h"
#include "Kernels/AbsFFT/Kernels_AbsFFT.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/AudioPipeline/AudioPipelineOptions.h"
#include "Tools/AudioFormatUtils.h"
#include "IO/AudioFileLoader.h"
#include "AudioConstants.h"
#include "AudioTemplate.h"

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
    : m_num_windows(windows)
    , m_num_frequencies(frequencies)
    , m_bytes_per_spectrum(Kernels::align_int_up<PA_ALIGNMENT>(frequencies * sizeof(float)))
    , m_spectrogram(windows * (m_bytes_per_spectrum / sizeof(float)))
{}


AudioTemplate load_audio_template(const std::string& filename, size_t sample_rate){
    QAudioFormat output_audio_format;
    output_audio_format.setChannelCount(1);
    output_audio_format.setSampleRate((int)sample_rate);
    set_sample_format_to_float(output_audio_format);

    AudioTemplate audio_template;

    GlobalThreadPools::qt_worker_threadpool().run_and_wait([&]{
        AudioFileLoader loader(nullptr, filename, output_audio_format);
        const auto ret = loader.load_full_audio();
        const float* data = reinterpret_cast<const float*>(std::get<0>(ret));
        size_t num_samples = std::get<1>(ret) / sizeof(float);
    //    cout << "num_samples = " << num_samples << endl;

        if (data == nullptr){
            return;
        }

        size_t num_frequencies = NUM_FFT_SAMPLES / 2;
        AlignedVector<float> input_buffer(NUM_FFT_SAMPLES);
        AlignedVector<float> output_buffer(num_frequencies);

        size_t num_windows = 0;

        const float* kaiser_weights = GlobalSettings::instance().AUDIO_PIPELINE->USE_KAISER_WINDOW
            ? Kernels::KaiserWindow::get_kaiser_window_k(FFT_LENGTH_POWER_OF_TWO)
            : nullptr;

        // If sample count < FFT input requirement, we pad zeros in the end to do one FFT.
        // Otherwise, we don't pad zeros and compute FFT as much as possible using fixed
        // window step.
        if (num_samples < NUM_FFT_SAMPLES){
            num_windows = 1;
            audio_template = AudioTemplate(num_frequencies, 1);

            memset(input_buffer.data(), 0, sizeof(float) * NUM_FFT_SAMPLES);
            memcpy(input_buffer.data(), data, sizeof(float) * num_samples);
            Kernels::AbsFFT::fft_abs(
                FFT_LENGTH_POWER_OF_TWO,
                output_buffer.data(),
                input_buffer.data(),
                kaiser_weights
            );
            memcpy(audio_template.get_window(0), output_buffer.data(), sizeof(float) * num_frequencies);
        }else{
            num_windows = (num_samples - NUM_FFT_SAMPLES) / FFT_SLIDING_WINDOW_STEP + 1;
            audio_template = AudioTemplate(num_frequencies, num_windows);

            for (size_t i = 0, start = 0; start+NUM_FFT_SAMPLES <= num_samples; i++, start += FFT_SLIDING_WINDOW_STEP){
                assert(i < num_windows);
                memcpy(input_buffer.data(), data + start, sizeof(float) * NUM_FFT_SAMPLES);
                Kernels::AbsFFT::fft_abs(
                    FFT_LENGTH_POWER_OF_TWO,
                    output_buffer.data(),
                    input_buffer.data(),
                    kaiser_weights
                );
                memcpy(audio_template.get_window(i), output_buffer.data(), sizeof(float) * num_frequencies);
            }
        }


        std::stringstream ss;
        ss << "Built audio template with sample rate " << sample_rate << ", " << num_windows << " windows and " << num_frequencies <<
            " frequencies from " << filename;
        global_logger_tagged().log(ss.str());
    });
    
    return audio_template;
}


















}
