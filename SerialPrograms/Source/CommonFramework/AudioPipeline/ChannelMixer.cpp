/*  Audio Channel Mixer - Matrix Builder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cstdint>
#include <QAudioFormat>
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "ChannelMixer.h"

namespace PokemonAutomation{

namespace{

using AudioChannelPosition = QAudioFormat::AudioChannelPosition;


//
//  The logical mix groups that channel positions belong to.
//
enum class MixFamily{
    LEFT_FRONT,
    RIGHT_FRONT,
    CENTER,
    LEFT_SURROUND,
    RIGHT_SURROUND,
    LFE,
    LFE2,
    OTHER,
};



MixFamily mix_family(AudioChannelPosition position){
    switch (position){
    case AudioChannelPosition::FrontLeft:
    case AudioChannelPosition::FrontLeftOfCenter:
        return MixFamily::LEFT_FRONT;
    case AudioChannelPosition::FrontRight:
    case AudioChannelPosition::FrontRightOfCenter:
        return MixFamily::RIGHT_FRONT;
    case AudioChannelPosition::FrontCenter:
        return MixFamily::CENTER;
    case AudioChannelPosition::BackLeft:
    case AudioChannelPosition::SideLeft:
        return MixFamily::LEFT_SURROUND;
    case AudioChannelPosition::BackRight:
    case AudioChannelPosition::SideRight:
        return MixFamily::RIGHT_SURROUND;
    case AudioChannelPosition::LFE:
        return MixFamily::LFE;
    case AudioChannelPosition::LFE2:
        return MixFamily::LFE2;
    default:
        return MixFamily::OTHER;
    }
}



//
//  Weight for the channel at input position `in` contributing to the channel at output
//  position `out`. A position that has its own channel on the other side always passes
//  through there (gain 1); the 0.7 downmix / upmix coefficients only apply when the
//  destination layout has no channel of that family.
//
float mix_gain(
    AudioChannelPosition in, AudioChannelPosition out,
    const std::vector<bool>& in_has,
    const std::vector<bool>& out_has,
    const std::vector<size_t>& out_family_count,
    bool in_mono, bool out_mono
){
    if (in_mono){
        switch (mix_family(out)){
        case MixFamily::LEFT_FRONT:
        case MixFamily::RIGHT_FRONT:
        case MixFamily::CENTER:
            return 1.0f;
        default:
            return 0.0f;
        }
    }
    if (out_mono){
        switch (mix_family(in)){
        case MixFamily::LEFT_FRONT:
        case MixFamily::RIGHT_FRONT:
        case MixFamily::CENTER:
            return 0.5f;
        default:
            return 0.0f;
        }
    }
    if (in == out){
        return 1.0f;
    }
    const MixFamily in_family = mix_family(in);
    const MixFamily out_family = mix_family(out);
    //  Same-family cross-fill: e.g. 7.1 -> 5.1: BL <- 0.7 * SL, or 5.1 -> 7.1: SL <- 0.7 * BL.
    if (in_family == out_family && (!out_has[in] || !in_has[out])){
        switch (in_family){
        case MixFamily::LEFT_FRONT:
        case MixFamily::RIGHT_FRONT:
        case MixFamily::LEFT_SURROUND:
        case MixFamily::RIGHT_SURROUND:
            return 0.7f;
        default:
            break;
        }
    }
    //  Front bleed: the input family has no channel in the output at all.
    if (out_family_count[(size_t)in_family] == 0){
        //  e.g. 5.1 -> 2.0: L <- 0.7 * C + 0.7 * Ls
        if ((in_family == MixFamily::CENTER && (out_family == MixFamily::LEFT_FRONT || out_family == MixFamily::RIGHT_FRONT))
            || (in_family == MixFamily::LEFT_SURROUND && out_family == MixFamily::LEFT_FRONT)
            || (in_family == MixFamily::RIGHT_SURROUND && out_family == MixFamily::RIGHT_FRONT)){
            return 0.7f;
        }
    }
    //  Center upmix: e.g. 2.0 -> 5.1: C <- 0.7 * (L + R) / 2
    if (out_family == MixFamily::CENTER && !in_has[AudioChannelPosition::FrontCenter]){
        if (in_family == MixFamily::LEFT_FRONT || in_family == MixFamily::RIGHT_FRONT){
            return 0.35f;
        }
    }
    //  LFE, LFE2, and top/bottom/back-center channels only pass through exactly.
    return 0.0f;
}



//
//  Maps each interleaved channel index of `format` to its position. Unknown channels
//  (present in the frame but not named by the config) stay UnknownPosition.
//
//  Returns false only when the format has a non-positive channel count.
//
bool resolve_positions(const QAudioFormat& format, std::vector<AudioChannelPosition>& out){
    const int count = format.channelCount();
    if (count <= 0){
        return false;
    }
    out.assign(count, AudioChannelPosition::UnknownPosition);
    const quint32 config = format.channelConfig();
    if (config == QAudioFormat::ChannelConfigUnknown){
        return true;
    }
    for (int p = 1; p < QAudioFormat::NChannelPositions; p++){
        if (config & (1u << p)){
            int offset = format.channelOffset((AudioChannelPosition)p);
            if (offset >= 0 && offset < count){
                out[offset] = (AudioChannelPosition)p;
            }
        }
    }
    return true;
}



}



std::vector<float> build_channel_mixer_matrix(const QAudioFormat& input, const QAudioFormat& output){
    std::vector<AudioChannelPosition> in_positions, out_positions;
    if (!resolve_positions(input, in_positions) || !resolve_positions(output, out_positions)){
        return {};
    }
    const bool in_unknown = input.channelConfig() == QAudioFormat::ChannelConfigUnknown;
    const bool out_unknown = output.channelConfig() == QAudioFormat::ChannelConfigUnknown;
    if (in_unknown || out_unknown){
        return {};
    }

    const size_t in_count = in_positions.size();
    const size_t out_count = out_positions.size();
    const bool in_mono = in_count == 1 && in_positions[0] == AudioChannelPosition::FrontCenter;
    const bool out_mono = out_count == 1 && out_positions[0] == AudioChannelPosition::FrontCenter;

    const size_t np = QAudioFormat::NChannelPositions;
    std::vector<bool> in_has(np, false), out_has(np, false);
    std::vector<size_t> out_family_count(8, 0);
    for (AudioChannelPosition p : in_positions){
        if (p != AudioChannelPosition::UnknownPosition){
            in_has[p] = true;
        }
    }
    for (AudioChannelPosition p : out_positions){
        if (p != AudioChannelPosition::UnknownPosition){
            out_has[p] = true;
            out_family_count[(size_t)mix_family(p)]++;
        }
    }

    std::vector<float> matrix(in_count * out_count, 0.0f);
    for (size_t o = 0; o < out_count; o++){
        const AudioChannelPosition out_position = out_positions[o];
        if (out_position == AudioChannelPosition::UnknownPosition){
            continue;
        }
        for (size_t i = 0; i < in_count; i++){
            const AudioChannelPosition in_position = in_positions[i];
            if (in_position == AudioChannelPosition::UnknownPosition){
                continue;
            }
            matrix[i + in_count * o] = mix_gain(
                in_position, out_position, in_has, out_has, out_family_count, in_mono, out_mono
            );
        }
    }
    return matrix;
}



namespace{


//
//  Builds a channel mixer matrix from a pair of QAudioFormat channel configs and verifies
//  that it matches the expected matrix. A config of 0 means "no channel layout"
//  (channel count only, all positions unknown), which must fail with an empty matrix.
//
class MixerMatrixTest : public UnitTest{
public:
    MixerMatrixTest(
        const std::string& name,
        uint32_t in_config,
        uint32_t out_config,
        std::vector<float> expected
    );

    virtual UnitTestResult run(Logger&, CancellableScope&) const override;


private:
    uint32_t m_in_config;
    uint32_t m_out_config;
    std::vector<float> m_expected;
};



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
    : UnitTest("CommonFramework::AudioPipeline - " + name)
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



void add_tests_ChannelMixerMatrix(UnitTestDatabase& database){
    //  Channel mixer matrix builder (industry-standard coefficients).
    using CC = QAudioFormat::ChannelConfig;
    database.add<MixerMatrixTest>(
        "MixerMatrix(Mono->Stereo)", CC::ChannelConfigMono, CC::ChannelConfigStereo,
        std::vector<float>{1.0f, 1.0f}
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(Stereo->Mono)", CC::ChannelConfigStereo, CC::ChannelConfigMono,
        std::vector<float>{0.5f, 0.5f}
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(3.0->Stereo)", CC::ChannelConfig3Dot0, CC::ChannelConfigStereo,
        std::vector<float>{
            1.0f, 0.0f, 0.7f,
            0.0f, 1.0f, 0.7f
        }
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(Stereo->3.0)", CC::ChannelConfigStereo, CC::ChannelConfig3Dot0,
        std::vector<float>{
            1.0f, 0.0f,
            0.0f, 1.0f,
            0.35f, 0.35f
        }
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(5.1->Stereo)", CC::ChannelConfigSurround5Dot1, CC::ChannelConfigStereo,
        std::vector<float>{
            1.0f, 0.0f, 0.7f, 0.0f, 0.7f, 0.0f,
            0.0f, 1.0f, 0.7f, 0.0f, 0.0f, 0.7f
        }
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(Stereo->5.1)", CC::ChannelConfigStereo, CC::ChannelConfigSurround5Dot1,
        std::vector<float>{
            1.0f, 0.0f,
            0.0f, 1.0f,
            0.35f, 0.35f,
            0.0f, 0.0f,
            0.0f, 0.0f,
            0.0f, 0.0f
        }
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(5.1->Mono)", CC::ChannelConfigSurround5Dot1, CC::ChannelConfigMono,
        std::vector<float>{0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f}
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(2.1->Stereo)", CC::ChannelConfig2Dot1, CC::ChannelConfigStereo,
        std::vector<float>{
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f
        }
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(7.1->5.1)", CC::ChannelConfigSurround7Dot1, CC::ChannelConfigSurround5Dot1,
        std::vector<float>{
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.7f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.7f
        }
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(5.1->3.0)", CC::ChannelConfigSurround5Dot1, CC::ChannelConfig3Dot0,
        std::vector<float>{
            1.0f, 0.0f, 0.0f, 0.0f, 0.7f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.7f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f
        }
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(5.1->2.1)", CC::ChannelConfigSurround5Dot1, CC::ChannelConfig2Dot1,
        std::vector<float>{
            1.0f, 0.0f, 0.7f, 0.0f, 0.7f, 0.0f,
            0.0f, 1.0f, 0.7f, 0.0f, 0.0f, 0.7f,
            0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f
        }
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(5.1->7.1)", CC::ChannelConfigSurround5Dot1, CC::ChannelConfigSurround7Dot1,
        std::vector<float>{
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 0.7f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.7f
        }
    );
    std::vector<float> identity_5_1(36, 0.0f);
    for (size_t c = 0; c < 6; c++){
        identity_5_1[c + 6 * c] = 1.0f;
    }
    database.add<MixerMatrixTest>(
        "MixerMatrix(5.1->5.1)", CC::ChannelConfigSurround5Dot1, CC::ChannelConfigSurround5Dot1,
        identity_5_1
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(UnknownIn->Stereo)", 0, CC::ChannelConfigStereo,
        std::vector<float>{}
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(Stereo->UnknownOut)", CC::ChannelConfigStereo, 0,
        std::vector<float>{}
    );
}



}

