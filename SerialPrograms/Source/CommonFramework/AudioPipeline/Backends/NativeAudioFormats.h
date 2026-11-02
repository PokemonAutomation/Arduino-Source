/*  Native Audio Formats
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_NativeAudioFormats_H
#define PokemonAutomation_AudioPipeline_NativeAudioFormats_H

#include <vector>
#include <QAudioFormat>
#include "CommonFramework/AudioPipeline/AudioFormat.h"

namespace PokemonAutomation{



//  The standard channel layout for a channel count, e.g. 1 -> Mono,
//  2 -> Stereo, 6 -> 5.1. Returns ChannelConfigUnknown for counts with no
//  standard layout (0, or more than 8).
QAudioFormat::ChannelConfig standard_channel_config(size_t channels);



//  Native formats supported by an audio input device, and the format it runs
//  in right now.
struct NativeAudioFormats{
    std::vector<AudioFormat> formats;   //  the formats to offer
    AudioFormat current;                //  the format the device runs in right now
};


//  Returns native formats supported by an audio input device and the format
//  it runs in right now.
NativeAudioFormats native_input_formats(const NativeAudioInfo& device);



}
#endif
