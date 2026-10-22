/*  Audio Channel Mixer - Matrix Builder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_ChannelMixer_H
#define PokemonAutomation_AudioPipeline_ChannelMixer_H

#include <vector>

class QAudioFormat;

namespace PokemonAutomation{

class UnitTestDatabase;


//
//  Builds the N x M channel mix weight matrix that converts the input format's channels
//  into the output format's channels.
//
//  The returned vector has input.channelCount() * output.channelCount() floats, laid out
//  one column per output channel, matching Kernels::AudioChannelMixer::mix_channels().
//  The weight for input channel i contributing to output channel o is at index
//  (i + in_channels * o).
//
//  Channel identities come from the channel configs (QAudioFormat::channelConfig() and
//  channelOffset()). Mixing follows the conventional "industry standard" coefficients
//  (ITU-R BS.1116-1 style downmixes). A position that exists on both sides always passes
//  through to itself (gain 1); the 0.7 coefficients only apply to positions that the
//  destination layout does not have:
//
//    - Same position on both sides: gain of 1.
//    - Mono -> front channels: gain of 1 (stereo upmix duplicates the channel).
//    - L/R -> mono: 0.5 each.
//    - Center -> L/R (when the output has no center channel): 0.7 each.
//    - Back/side surrounds -> the corresponding front channel (when the output has no
//      back/side surround on that side): 0.7.
//    - L/R -> center (upmix, when the input has no center): 0.7 * (L + R) / 2,
//      i.e. 0.35 per front channel.
//    - LFE passes through to LFE only; it is dropped otherwise.
//    - Front-left/right-of-center follow their front channel, and back/side surround
//      pairs follow each other (e.g. SideLeft <-> BackLeft), at 0.7 when the exact
//      position is missing on either side.
//    - Top, bottom, and back-center channels never mix (dropped in downmix, silent in
//      upmix).
//
//  When a format's channel layout is unknown (channelConfig() == ChannelConfigUnknown),
//  the standard layout for the channel count is assumed: 1 = mono, 2 = stereo,
//  3 = 2.1, 4 = 3.1, 5 = 5.0, 6 = 5.1, 7 = 7.0, 8 = 7.1.
//
//  An empty vector is returned on failure:
//    - Either format has a non-positive channel count, or
//    - Either format's channel layout is unknown and its channel count has no standard
//      fallback layout.
//  A channel that exists in the frame but has no position in its config contributes
//  nothing (its matrix row is all zeros), and an output channel with no position
//  receives nothing (its column is all zeros).
//
//  No renormalization is applied: the standard coefficients are fixed, so a full-scale
//  multichannel source can exceed full-scale in a narrow downmix.
//
std::vector<float> build_channel_mixer_matrix(const QAudioFormat& input, const QAudioFormat& output);



//
//  Registers unit tests for build_channel_mixer_matrix.
//
void add_tests_ChannelMixerMatrix(UnitTestDatabase& database);



}
#endif
