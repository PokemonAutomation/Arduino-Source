/*  Audio Pipeline - ChannelMixer Matrix Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QAudioFormat>
#include "CommonFramework/AudioPipeline/ChannelMixer.h"
#include "Kernels_ChannelMixer_Matrix_TestInfra.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{



namespace{


void apply_config(QAudioFormat& format, uint32_t config){
    format.setSampleFormat(QAudioFormat::SampleFormat::Float);
    format.setSampleRate(48000);
    if (config == QAudioFormat::ChannelConfigUnknown){
        //  No channel layout: channel count only, all positions unknown.
        format.setChannelCount(6);
        return;
    }
    format.setChannelConfig(QAudioFormat::ChannelConfig(config));
}



}




MixerMatrixTest::MixerMatrixTest(
    const std::string& name,
    uint32_t in_config,
    uint32_t out_config,
    std::vector<float> expected
)
    : UnitTest("Kernels::AudioPipeline - " + name)
    , m_in_config(in_config)
    , m_out_config(out_config)
    , m_expected(std::move(expected))
{}

UnitTestResult MixerMatrixTest::run(Logger&, CancellableScope&) const{
    QAudioFormat in_format, out_format;
    apply_config(in_format, m_in_config);
    apply_config(out_format, m_out_config);

    std::vector<float> actual = build_channel_mixer_matrix(in_format, out_format);

    if (actual.size() != m_expected.size()){
        return "Matrix size mismatch: got " + std::to_string(actual.size()) + ", expected " +
            std::to_string(m_expected.size()) + ".";
    }
    for (size_t i = 0; i < actual.size(); i++){
        if (actual[i] != m_expected[i]){
            return "Mismatch at index " + std::to_string(i) + ": got " +
                std::to_string(actual[i]) + ", expected " + std::to_string(m_expected[i]) + ".";
        }
    }
    return true;
}



}
}
}
