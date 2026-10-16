/*  Audio Channel Mixer - Matrix Builder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QtGlobal>
#include <QAudioFormat>
#include "ChannelMixer.h"

namespace PokemonAutomation{

#if QT_VERSION_MAJOR == 5
//
//  Qt 5's QAudioFormat has no per-channel position info (channelConfig() is a Qt 6 API),
//  so channel identities are unknown and no standard matrix can be built.
//
std::vector<float> build_channel_mixer_matrix(const QAudioFormat&, const QAudioFormat&){
    return {};
}

#else

namespace{

using P = QAudioFormat::AudioChannelPosition;


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



MixFamily mix_family(P position){
    switch (position){
    case P::FrontLeft:
    case P::FrontLeftOfCenter:
        return MixFamily::LEFT_FRONT;
    case P::FrontRight:
    case P::FrontRightOfCenter:
        return MixFamily::RIGHT_FRONT;
    case P::FrontCenter:
        return MixFamily::CENTER;
    case P::BackLeft:
    case P::SideLeft:
        return MixFamily::LEFT_SURROUND;
    case P::BackRight:
    case P::SideRight:
        return MixFamily::RIGHT_SURROUND;
    case P::LFE:
        return MixFamily::LFE;
    case P::LFE2:
        return MixFamily::LFE2;
    default:
        return MixFamily::OTHER;
    }
}



//
//  Weight for the channel at input position `in` contributing to the channel at output
//  position `out`. A position that has its own channel on the other side always passes
//  through there (gain 1); the 0.7 downmix / upmix coefficients only apply when the
//  destination layout has no channel of that family, matching FFmpeg's automatic rematrix.
//
float mix_gain(
    P in, P out,
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
    if (out_family == MixFamily::CENTER && !in_has[P::FrontCenter]){
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
bool resolve_positions(const QAudioFormat& format, std::vector<P>& out){
    const int count = format.channelCount();
    if (count <= 0){
        return false;
    }
    out.assign(count, P::UnknownPosition);
    const quint32 config = format.channelConfig();
    if (config == QAudioFormat::ChannelConfigUnknown){
        return true;
    }
    for (int p = 1; p < QAudioFormat::NChannelPositions; p++){
        if (config & (1u << p)){
            int offset = format.channelOffset((P)p);
            if (offset >= 0 && offset < count){
                out[offset] = (P)p;
            }
        }
    }
    return true;
}



}



std::vector<float> build_channel_mixer_matrix(const QAudioFormat& input, const QAudioFormat& output){
    std::vector<P> in_positions, out_positions;
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
    const bool in_mono = in_count == 1 && in_positions[0] == P::FrontCenter;
    const bool out_mono = out_count == 1 && out_positions[0] == P::FrontCenter;

    const size_t np = QAudioFormat::NChannelPositions;
    std::vector<bool> in_has(np, false), out_has(np, false);
    std::vector<size_t> out_family_count(8, 0);
    for (P p : in_positions){
        if (p != P::UnknownPosition){
            in_has[p] = true;
        }
    }
    for (P p : out_positions){
        if (p != P::UnknownPosition){
            out_has[p] = true;
            out_family_count[(size_t)mix_family(p)]++;
        }
    }

    std::vector<float> matrix(in_count * out_count, 0.0f);
    for (size_t o = 0; o < out_count; o++){
        const P out_position = out_positions[o];
        if (out_position == P::UnknownPosition){
            continue;
        }
        for (size_t i = 0; i < in_count; i++){
            const P in_position = in_positions[i];
            if (in_position == P::UnknownPosition){
                continue;
            }
            matrix[i + in_count * o] = mix_gain(
                in_position, out_position, in_has, out_has, out_family_count, in_mono, out_mono
            );
        }
    }
    return matrix;
}

#endif


}
