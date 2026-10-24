/*  Audio Pipeline - ChannelMixer Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <random>
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Common/Cpp/Containers/GuardPageBuffer.h"
#include "Kernels_ChannelMixer_TestInfra.h"
#include "Kernels_ChannelMixer.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{




namespace{



class RecordingWorkingAudioListener : public WorkingAudioStreamListener{
public:
    RecordingWorkingAudioListener(size_t sample_rate, size_t channels)
        : WorkingAudioStreamListener(sample_rate, channels)
    {}

    virtual void push_samples(size_t samples, size_t stride, const float* data) override{
        if (m_data.empty()){
            m_stride = stride;
            m_data = AlignedVector<float>(channels() * stride);
        }
        for (size_t c = 0; c < channels(); c++){
            for (size_t s = 0; s < samples; s++){
                m_data[c * m_stride + s] = data[c * stride + s];
            }
        }
        m_samples_pushed += samples;
    }

    const AlignedVector<float>& data() const{
        return m_data;
    }
    size_t stride() const{
        return m_stride;
    }
    size_t samples_pushed() const{
        return m_samples_pushed;
    }


private:
    AlignedVector<float> m_data;
    size_t m_stride = 0;
    size_t m_samples_pushed = 0;
};



//  Random integer samples, all exactly representable in a float. The buffer
//  ends right before a guard page, so a read past the end faults.
GuardPageBuffer<float> make_random_input(size_t channels, size_t samples){
    GuardPageBuffer<float> input(channels * samples);
    std::mt19937 generator(42);
    std::uniform_int_distribution<long long> distribution(-128, 128);
    for (size_t c = 0; c < input.size(); c++){
        input[c] = (float)distribution(generator);
    }
    return input;
}



}




MixerIdentityTest::MixerIdentityTest(size_t channels, size_t samples)
    : UnitTest(
        "Kernels::AudioPipeline - MixerIdentity(" +
        std::to_string(channels) + "ch x " + std::to_string(samples) + ")"
    )
    , m_channels(channels)
    , m_samples(samples)
{}
UnitTestResult MixerIdentityTest::run(Logger& logger, CancellableScope& scope) const{
    std::vector<float> matrix(m_channels * m_channels, 0.0f);
    for (size_t c = 0; c < m_channels; c++){
        matrix[c + m_channels * c] = 1.0f;
    }

    GuardPageBuffer<float> input = make_random_input(m_channels, m_samples);

    const size_t sample_rate = 44100;

    ChannelMixer mixer(logger, sample_rate, m_channels, m_channels, std::move(matrix));
    RecordingWorkingAudioListener listener(sample_rate, m_channels);
    mixer.add_listener(listener);

    mixer.push_samples(m_samples, m_samples, input.data());

    if (listener.samples_pushed() != m_samples){
        return "Received " + std::to_string(listener.samples_pushed()) + " samples, expected " +
            std::to_string(m_samples) + ".";
    }

    //  Check for errors.
    for (size_t c = 0; c < m_channels; c++){
        for (size_t s = 0; s < m_samples; s++){
            float expected = input[c * m_samples + s];
            float actual = listener.data()[c * listener.stride() + s];
            if (expected != actual){
                return "Mismatch at (" + std::to_string(c) + ", " + std::to_string(s) + "): " +
                    std::to_string(expected) + " != " + std::to_string(actual);
            }
        }
    }

    return true;
}




MixerRoundTripTest::MixerRoundTripTest(size_t samples)
    : UnitTest(
        "Kernels::AudioPipeline - Mixer_1to2_2to1(" + std::to_string(samples) + ")"
    )
    , m_samples(samples)
{}
UnitTestResult MixerRoundTripTest::run(Logger& logger, CancellableScope& scope) const{
    //  1 -> 2: Averaging a single channel is the channel itself.
    std::vector<float> duplicate_matrix{1.0f, 1.0f};
    //  2 -> 1: Average the two channels.
    std::vector<float> average_matrix{0.5f, 0.5f};

    GuardPageBuffer<float> input = make_random_input(1, m_samples);

    const size_t sample_rate = 44100;

    ChannelMixer mixer0(logger, sample_rate, 1, 2, duplicate_matrix);
    ChannelMixer mixer1(logger, sample_rate, 2, 1, average_matrix);
    RecordingWorkingAudioListener middle(sample_rate, 2);
    RecordingWorkingAudioListener output(sample_rate, 1);
    mixer0.add_listener(mixer1);
    mixer0.add_listener(middle);
    mixer1.add_listener(output);

    mixer0.push_samples(m_samples, m_samples, input.data());

    if (middle.samples_pushed() != m_samples){
        return "Received " + std::to_string(middle.samples_pushed()) + " samples, expected " +
            std::to_string(m_samples) + ".";
    }
    if (output.samples_pushed() != m_samples){
        return "Received " + std::to_string(output.samples_pushed()) + " samples, expected " +
            std::to_string(m_samples) + ".";
    }

    //  Check for errors.
    for (size_t s = 0; s < m_samples; s++){
        float expected = input[s];
        for (size_t c = 0; c < 2; c++){
            float actual = middle.data()[c * middle.stride() + s];
            if (expected != actual){
                return "1->2 mismatch at (" + std::to_string(c) + ", " + std::to_string(s) + "): " +
                    std::to_string(expected) + " != " + std::to_string(actual);
            }
        }
        float round_trip = output.data()[s];
        if (expected != round_trip){
            return "2->1 mismatch at " + std::to_string(s) + ": " +
                std::to_string(expected) + " != " + std::to_string(round_trip);
        }
    }

    return true;
}




}
}
}
