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

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{

namespace{

//  The matrix is laid out weight(in i -> out o) at index i + in_channels * o;
//  it can only be the identity for a square one. The matrices that reach
//  this node are program-generated, so exact float comparison is safe.
bool is_identity(const std::vector<float>& matrix, size_t channels){
    if (matrix.size() != channels * channels){
        return false;
    }
    for (size_t i = 0; i < channels; i++){
        for (size_t o = 0; o < channels; o++){
            const float w = matrix[i + channels * o];
            if ((i == o ? w != 1.0f : w != 0.0f)){
                return false;
            }
        }
    }
    return true;
}

}


struct WorkingAudio2WorkingAudio::Data{
    WorkingAudioNode* head = nullptr;
    WorkingAudioNode* tail = nullptr;

    std::unique_ptr<ChannelMixer> mixer;
    std::unique_ptr<AudioResampler::AudioResampler> resampler;

    //  Only in use when the chain is empty (same rate, same channel
    //  count, identity matrix): the node itself is the pass-through.
    ListenerSet<WorkingAudioStreamListener> listeners;
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
    std::vector<WorkingAudioNode*> nodes;

    Data& self = *m_data;
    //  A square identity matrix is a no-op, so the mixer is only needed
    //  when the channel counts differ or the matrix actually mixes.
    const bool need_mixer = in_channels != out_channels
        || !is_identity(weight_matrix, in_channels);
    if (in_channels >= out_channels){
        //  Mix first (at the input rate), then resample.
        if (need_mixer){
            self.mixer.reset(new ChannelMixer(
                logger,
                in_sample_rate,
                in_channels, out_channels,
                std::move(weight_matrix),
                buffer_size
            ));
            nodes.emplace_back(self.mixer.get());
        }
        if (in_sample_rate != out_sample_rate){
            self.resampler.reset(new AudioResampler::AudioResampler(
                logger,
                in_sample_rate, out_sample_rate,
                out_channels,
                buffer_size
            ));
            nodes.emplace_back(self.resampler.get());
        }
    }else{
        //  Resample first (at the cheaper input channel count), then mix.
        if (in_sample_rate != out_sample_rate){
            self.resampler.reset(new AudioResampler::AudioResampler(
                logger,
                in_sample_rate, out_sample_rate,
                in_channels,
                buffer_size
            ));
            nodes.emplace_back(self.resampler.get());
        }
        if (need_mixer){
            self.mixer.reset(new ChannelMixer(
                logger,
                out_sample_rate,
                in_channels, out_channels,
                std::move(weight_matrix),
                buffer_size
            ));
            nodes.emplace_back(self.mixer.get());
        }
    }
    populate_pointers(logger, nodes);
}

void WorkingAudio2WorkingAudio::populate_pointers(
    Logger& logger,
    const std::vector<WorkingAudioNode*>& nodes
){
    logger.log("WorkingAudio2WorkingAudio: Nodes = " + std::to_string(nodes.size()));

    Data& self = *m_data;
    for (size_t c = 0; c < nodes.size(); c++){
        if (c > 0){
            nodes[c - 1]->add_listener(*nodes[c]);
        }
    }
    if (!nodes.empty()){
        self.head = nodes.front();
        self.tail = nodes.back();
    }
}

void WorkingAudio2WorkingAudio::add_listener(WorkingAudioStreamListener& listener){
    Data& self = *m_data;
    if (self.tail){
        self.tail->add_listener(listener);
    }else{
        self.listeners.add(listener);
    }
}
void WorkingAudio2WorkingAudio::remove_listener(WorkingAudioStreamListener& listener){
    Data& self = *m_data;
    if (self.tail){
        self.tail->remove_listener(listener);
    }else{
        self.listeners.remove(listener);
    }
}

void WorkingAudio2WorkingAudio::push_samples(size_t samples, size_t stride, const float* data){
    Data& self = *m_data;
    if (self.head){
        self.head->push_samples(samples, stride, data);
    }else{
        self.listeners.run_method(
            &WorkingAudioStreamListener::push_samples,
            samples, stride, data
        );
    }
}




}
}
}
