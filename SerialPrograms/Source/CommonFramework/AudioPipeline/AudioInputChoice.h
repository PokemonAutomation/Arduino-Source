/*  Audio Input Choice
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioInputChoice_H
#define PokemonAutomation_AudioPipeline_AudioInputChoice_H

#include <string>
#include <vector>
#include "AudioFormat.h"

namespace PokemonAutomation{


//
//  An entry in the audio input format dropdown. It carries the shape the
//  pipeline consumes ("format") and, for misreporting capture cards, how
//  that shape relates to what the device physically emits
//  ("physical_format").
//
//  Native choices have physical == format. Some capture
//  cards only expose a 1ch x 96000 stream although they actually deliver
//  two 48 kHz channels interleaved; for those, in addition to the native
//  choice, two reinterpreted choices read the same physical stream as
//  2ch x 48000: "Interleaved Stereo L/R" (the physical channel order) and
//  "Interleaved Stereo R/L" (swapped).
//
class AudioInputChoice{
public:
    ~AudioInputChoice();
    AudioInputChoice(const AudioInputChoice& x);
    AudioInputChoice& operator=(const AudioInputChoice& x);

public:
    AudioInputChoice();                     //  invalid
    AudioInputChoice(AudioFormat format);   //  native: physical == format

    static AudioInputChoice reinterpreted(
        AudioFormat format, AudioFormat physical, bool swap_channels
    );

    bool is_valid() const;
    explicit operator bool() const;

    bool operator==(const AudioInputChoice& x) const;

    const AudioFormat& format() const;          //  what the pipeline consumes
    const AudioFormat& physical_format() const; //  what the device emits
    bool is_reinterpreted() const;
    bool swap_channels() const;

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

//  Builds the input format choices for a device from its supported formats
//  and its preferred format.
//
//  1. Deduplicates formats that has the same shape but only differ in
//  sample dtype.
//  2. Sorted by channel count, then sample rate, both descending.
//  3. A 1ch x 96000 entry is followed by its two reinterpreted stereo
//     choices, "Interleaved Stereo L/R" then "Interleaved Stereo R/L".
//  4. `AudioInputChoiceList.preferred_index` is the exact match of
//     `preferred`, or the highest-scoring choice when nothing matches.
AudioInputChoiceList input_choices(
    std::vector<AudioFormat> supported,
    const AudioFormat& preferred
);




}
#endif
