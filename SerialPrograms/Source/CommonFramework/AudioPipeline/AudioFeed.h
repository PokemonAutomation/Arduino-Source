/*  Audio Feed Interface
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AudioFeedInterface_H
#define PokemonAutomation_AudioFeedInterface_H

#include <memory>
#include <vector>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/AlignedVector.h"

namespace PokemonAutomation{

//  The result of one FFT computation, an array of the magnitudes of different
//  frequencies.
//  Each spectrum is computed using a sliding window on the incoming audio stream.
//  It has a stamp to denote which window it is from.
class AudioSpectrum{
public:
    //  The stamp to denote which audio window it is from.
    //  the stamp starts at 0 and becomes larger for later windows in the stream.
    size_t stamp = 0;

    size_t sample_rate;

    //  The frequency magnitudes from FFT. The order in the vector is from lower to
    //  higher frequencies.
    std::shared_ptr<const AlignedVector<float>> magnitudes;

    AudioSpectrum(size_t s, size_t rate, std::shared_ptr<const AlignedVector<float>> m)
        : stamp(s)
        , sample_rate(rate)
        , magnitudes(std::move(m))
    {}
};

//  Define basic interface of an audio feed to be used by programs or other services.
//  All the functions in the interface should be thread safe.
class AudioFeed{
public:
    //  Reset the video. Note that this may return early.
    virtual void reset() = 0;

    //  Return all the spectrums with stamps greater or equal to `startingStamp`
    //  Returned spectrums are ordered from newest (largest timestamp) to oldest (smallest timestamp) in the vector.
    virtual std::vector<AudioSpectrum> spectrums_since(size_t startingStamp) = 0;

    //  Return a specific number of latest spectrums.
    //  Returned spectrums are ordered from newest (largest timestamp) to oldest (smallest timestamp) in the vector.
    virtual std::vector<AudioSpectrum> spectrums_latest(size_t numLatestSpectrums) = 0;

    //  Add visual overlay to the spectrums starting at `startingStamp` and before `endStamp` with `color`.
    virtual void add_overlay(size_t startingStamp, size_t endStamp, Color color) = 0;
};



}
#endif
