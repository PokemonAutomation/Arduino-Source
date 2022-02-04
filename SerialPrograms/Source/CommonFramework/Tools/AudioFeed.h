/*  Audio Feed Interface
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AudioFeedInterface_H
#define PokemonAutomation_AudioFeedInterface_H

#include <vector>
#include <memory>

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
    //  Can call from anywhere.
    virtual void async_reset_audio() = 0;

    //  Return all the computed spectrums which stamps are greater
    //  or equal to `startingStamp`
    virtual void spectrums_since(size_t startingStamp, std::vector<std::shared_ptr<AudioSpectrum>>& spectrums) = 0;

    //  Return the latest spectrums.
    virtual void spectrums_latest(size_t numLatestSpectrums, std::vector<std::shared_ptr<AudioSpectrum>>& spectrums) = 0;
};



}
#endif
