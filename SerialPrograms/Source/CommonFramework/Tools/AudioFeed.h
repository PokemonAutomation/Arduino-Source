/*  Audio Feed Interface
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AudioFeedInterface_H
#define PokemonAutomation_AudioFeedInterface_H

#include <vector>
#include <memory>

#include "Common/Cpp/Color.h"

class QImage;

namespace PokemonAutomation{

//  The result of one FFT computation, an array of the magnitudes of different
//  frequencies.
//  Each spectrum is computed using a sliding window on the incoming audio stream.
//  It has a stamp to denote which window it is from.
struct AudioSpectrum{
    //  The stamp to denote which audio window it is from.
    //  the stamp starts at 0 and becomes larger for later windows in the stream.
    size_t stamp = 0;
    //  The frequency magnitudes from FFT. The order in the vector is from lower to
    //  higher frequencies.
    std::vector<float> magnitudes;

    AudioSpectrum(size_t s, const std::vector<float>& m) : stamp(s), magnitudes(m) {}
    AudioSpectrum(size_t s, std::vector<float>&& m) : stamp(s), magnitudes(std::move(m)) {}
    AudioSpectrum(const AudioSpectrum&) = default;
    AudioSpectrum(AudioSpectrum&&) = default;
};

//  Define basic interface of an audio feed to be used
//  by programs.
class AudioFeed{
public:
    //  Reset audio; Can be called from any thread.
    virtual void async_reset_audio() = 0;

    //  Return all the spectrums with stamps greater or equal to `startingStamp`
    //  Returned spectrums are ordered from newest (largest timestamp) to oldest (smallest timestamp) in the vector.
    virtual void spectrums_since(size_t startingStamp, std::vector<std::shared_ptr<AudioSpectrum>>& spectrums) = 0;

    //  Return a specific number of latest spectrums.
    //  Returned spectrums are ordered from newest (largest timestamp) to oldest (smallest timestamp) in the vector.
    virtual void spectrums_latest(size_t numLatestSpectrums, std::vector<std::shared_ptr<AudioSpectrum>>& spectrums) = 0;

    //  Add visual overlay to the spectrums starting at `startingStamp` and before `endStamp` with `color`.
    virtual void add_overlay(size_t startingStamp, size_t endStamp, Color color) = 0;
};



}
#endif
