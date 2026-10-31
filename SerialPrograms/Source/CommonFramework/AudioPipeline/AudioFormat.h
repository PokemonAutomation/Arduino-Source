/*  Audio Format
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioFormat_H
#define PokemonAutomation_AudioPipeline_AudioFormat_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "Kernels/AudioPipeline/Kernels_AudioPipeline.h"

class QAudioFormat;

class QAudioDevice;
using NativeAudioInfo = QAudioDevice;



namespace PokemonAutomation{


using AudioSampleFormat = Kernels::AudioPipeline::SampleFormat;
size_t sample_size(AudioSampleFormat format);

//  The pipeline's SampleFormat for a Qt sample format; throws for an
//  unknown Qt format.
AudioSampleFormat sample_format_of(const QAudioFormat& format);



class AudioFormat{
public:
    AudioFormat();
    AudioFormat(QAudioFormat native_format);

    AudioSampleFormat sample_format() const;
    size_t sample_rate() const;
    size_t channels() const;

    const QAudioFormat& native_format() const;
    QAudioFormat& native_format();

private:
    Pimpl<QAudioFormat> m_native_format;
};




}
#endif
