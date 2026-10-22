/*  Audio Stream Converters
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <memory>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Kernels/AudioResampler/Kernels_AudioResampler.h"
#include "Kernels_ChannelMixer.h"
#include "Kernels_WorkingAudio2WorkingAudio.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{


struct WorkingAudio2WorkingAudio::Data{
    WorkingAudioNode* head = nullptr;
    WorkingAudioNode* tail = nullptr;

    std::unique_ptr<ChannelMixer> mixer;
    std::unique_ptr<AudioResampler::AudioResampler> resampler;
};


WorkingAudio2WorkingAudio::~WorkingAudio2WorkingAudio() = default;

WorkingAudio2WorkingAudio::WorkingAudio2WorkingAudio(
    Logger& logger,
    size_t in_sample_rate, size_t out_sample_rate,
    size_t in_channels, size_t out_channels,
    std::vector<float> weight_matrix,
    size_t buffer_size
)
    : WorkingAudioNode(in_sample_rate, out_sample_rate, in_channels, out_channels)
    , m_data(CONSTRUCT_TOKEN)
{
    Data& self = *m_data;
    if (in_channels >= out_channels){
        self.mixer.reset(new ChannelMixer(
            in_sample_rate,
            in_channels, out_channels,
            std::move(weight_matrix),
            buffer_size
        ));
        self.head = self.mixer.get();
        self.tail = self.mixer.get();
        if (in_sample_rate != out_sample_rate){
            self.resampler.reset(new AudioResampler::AudioResampler(
                logger,
                in_sample_rate, out_sample_rate,
                out_channels,
                buffer_size
            ));
            self.tail = self.resampler.get();
        }
    }else{
        self.mixer.reset(new ChannelMixer(
            out_sample_rate,
            in_channels, out_channels,
            std::move(weight_matrix),
            buffer_size
        ));
        self.head = self.mixer.get();
        self.tail = self.mixer.get();
        if (in_sample_rate != out_sample_rate){
            self.resampler.reset(new AudioResampler::AudioResampler(
                logger,
                in_sample_rate, out_sample_rate,
                in_channels,
                buffer_size
            ));
            self.head = self.resampler.get();
        }
    }
    if (self.head != self.tail){
        self.head->add_listener(*self.tail);
    }
}

void WorkingAudio2WorkingAudio::add_listener(WorkingAudioStreamListener& listener){
    m_data->tail->add_listener(listener);
}
void WorkingAudio2WorkingAudio::remove_listener(WorkingAudioStreamListener& listener){
    m_data->tail->remove_listener(listener);
}

void WorkingAudio2WorkingAudio::push_samples(size_t samples, size_t stride, const float* data){
    m_data->head->push_samples(samples, stride, data);
}




}
}
}
