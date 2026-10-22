/*  Audio Device Info
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioInfo_H
#define PokemonAutomation_AudioPipeline_AudioInfo_H

#include <string>
#include <vector>
#include <QtConfig>
#include "Common/Cpp/Containers/Pimpl.h"

class QAudioFormat;

class QAudioDevice;
using NativeAudioInfo = QAudioDevice;



namespace PokemonAutomation{


enum class AudioSampleFormat{
    INVALID,
    UINT8,
    SINT16,
    SINT32,
    FLOAT32,
};
size_t sample_size(AudioSampleFormat format);


enum class AudioChannelFormat{
    NONE,
    MONO_48000,
    DUAL_44100,
    DUAL_48000,
    MONO_96000,
    INTERLEAVE_LR_96000,
    INTERLEAVE_RL_96000,
    END_LIST,
};
extern const char* AUDIO_FORMAT_LABELS[];

//  Convert our audio format enum to QAudioFormat.
void set_format(QAudioFormat& native_format, AudioChannelFormat format);

AudioSampleFormat get_sample_format(const QAudioFormat& native_format);



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


//
//  An entry in the audio input format dropdown. It carries the shape the
//  pipeline consumes ("format") and, for misreporting capture cards, how
//  that shape relates to what the device physically emits
//  ("physical_format").
//
//  Normal presets are native choices with physical == format. Some capture
//  cards only expose a 1ch x 96000 stream although they actually deliver
//  two 48 kHz channels interleaved; for those, in addition to the native
//  choice, two reinterpreted choices read the same physical stream as
//  2ch x 48000: "Interleaved Stereo L/R" (the physical channel order) and
//  "Interleaved Stereo R/L" (swapped).
//
class AudioInputChoice{
public:
    AudioInputChoice();                     //  invalid
    AudioInputChoice(AudioFormat format);    //  native: physical == format

    static AudioInputChoice reinterpreted(
        AudioFormat format, AudioFormat physical, bool swap_channels
    );

    bool is_valid() const;
    explicit operator bool() const;

    const AudioFormat& format() const;          //  what the pipeline consumes
    const AudioFormat& physical_format() const; //  what the device emits
    bool is_reinterpreted() const;
    bool swap_channels() const;

    //  "<channels> x <sample rate> Hz (<layout>)", e.g. "2 x 48,000 Hz
    //  (Stereo)" or "1 x 96,000 Hz (Mono)"; the layout name is only present
    //  for standard layouts (Mono, Stereo, 2.1, 3.1, 5.0, 5.1, 7.0, 7.1).
    std::string display_name() const;

private:
    AudioFormat m_format;
    AudioFormat m_physical;
    bool m_reinterpreted = false;
    bool m_swap_channels = false;
};

struct AudioInputChoiceList{
    std::vector<AudioInputChoice> choices;
    size_t preferred_index = 0;
};

//
//  Builds the input format choices for a device from its supported formats
//  (any order, duplicates allowed) and its preferred format:
//
//   - entries with the same (rate, channels, channel config) are collapsed
//     to one; when several sample formats are supported for the same shape,
//     the preferred sample format wins, otherwise the first seen does.
//   - the result is sorted by sample rate, then channel count.
//   - when the only supported format is 1ch x 96000 (the misreporting
//     capture-card case), two reinterpreted 2ch x 48000 choices are
//     appended after it (L/R first, then R/L).
//   - preferred_index is the index of the exact match of "preferred", or 0
//     when there is no match (or the list is empty).
//
AudioInputChoiceList input_choices(std::vector<AudioFormat> supported, const AudioFormat& preferred);




class AudioDeviceInfo{
public:
    ~AudioDeviceInfo();
    AudioDeviceInfo(const AudioDeviceInfo& x);
    void operator=(const AudioDeviceInfo& x);

public:
    AudioDeviceInfo();
    explicit AudioDeviceInfo(const std::string& device_name);

    explicit operator bool() const;

    const std::string& display_name() const;
    const std::string& device_name() const;

    const std::vector<AudioChannelFormat>& supported_formats() const;
    int preferred_format_index() const;
    QAudioFormat preferred_format() const;

    //  The device-driven format list (see input_choices()). Populated for
    //  input devices only; empty for output devices (the sink opens in its
    //  native format).
    const std::vector<AudioInputChoice>& supported_choices() const;
    size_t preferred_choice_index() const;

    const NativeAudioInfo& native_info() const;

    bool operator==(const AudioDeviceInfo& info) const;

    static std::vector<AudioDeviceInfo> all_input_devices();
    static std::vector<AudioDeviceInfo> all_output_devices();

private:
    struct Data;
    Pimpl<Data> m_body;
};





}
#endif
