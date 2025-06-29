/*  Spectrogram Matcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_SpectrogramMatcher_H
#define PokemonAutomation_CommonTools_SpectrogramMatcher_H

#include <cstddef>
#include <array>
#include <memory>
#include <vector>
#include <list>
#include "CommonFramework/AudioPipeline/AudioFeed.h"
#include "CommonFramework/AudioPipeline/AudioTemplate.h"

namespace PokemonAutomation{

class AudioSpectrum;

// Load an audio template from disk and use its spectrogram to match the
// spectrogram of the incoming audio stream.
class SpectrogramMatcher{
public:
    enum class Mode{
        // Don't do any processing on each window of spectrum, matching raw spectrums.
        RAW,
        // Do convolution on each window of spectrum with a peak detection kernel, before matching spectrums.
        SPIKE_CONV,
        // Do convolution on each window of spectrum with a Gaussian smooth kernel, before matching spectrums.
        // GAUSSIAN_CONV,
        // Average every 5 frequencies to reduce computation.
        AVERAGE_5,
    };

    // audioTemplate: the audio template for the audio stream to match against.
    //  Use AudioTemplate::loadAudioTemplate() to load a template from disk, or
    //  use AudioTemplateCache::instance().get_throw(audioResourceRelativePath, sample_rate) to get one from cache.
    // mode: which type of spectrogram filtering to use before the actual match.
    // sample_rate: audio sample rate.
    // low_frequency_filter: only match the frequencies above this threshold.
    // templateSubdivision: divide the template into how many sub-templates to match. <= 1 means no subdivision.
    SpectrogramMatcher(
        std::string name,
        AudioTemplate audioTemplate, Mode mode, size_t sample_rate,
        double low_frequency_filter, size_t templateSubdivision = 0
    );

    size_t sample_rate() const{ return m_sample_rate; }

    // Match the newest spectrums and return a match score.
    // Newer (larger timestamp) spectrums at beginning of `new_spectrums` while older (smaller
    // timestamp) spectrums at the end.
    // In invalid cases (internal error or not enough windows), return FLT_MAX
    float match(const std::vector<AudioSpectrum>& new_spectrums);

    // Pass some spectrums in but don't run match on them.
    // Used for skipping some spectrums to avoid unnecessary matching.
    // Newer (larger timestamp) spectrums at beginning of `new_spectrums` while older (smaller
    // timestamp) spectrums at the end.
    // Return true if there is no error.
    bool skip(const std::vector<AudioSpectrum>& new_spectrums);

    // Clear internal data to be used on another audio stream.
    void clear();

    // How many windows are used for matching.
    size_t numMatchedWindows() const { return m_numSpectrumsNeeded; }

    // Return latest timestamp from the stored audio spectrum stream.
    // Return SIZE_MAX if there is no stored spectrum yet.
    uint64_t latestTimestamp() const;

    // Return the scale found by the matcher to scale the input audio stream to best
    // match the template in the last `match()`.
    // Return 0.0f if the last scale is not available.
    float lastMatchedScale() const { return m_lastScale; }

private:
    void conv(const float* src, size_t num, float* dst);
    
    // The function to build `m_templateNorm`
    std::vector<float> buildTemplateNorm() const;

    // For a given sub-template, return its match score and scaling factor
    std::pair<float, float> match_sub_template(size_t sub_index) const;

    // Update internal data for the next new spectrum. Called by `update_to_new_spectrums()`.
    // Return true if there is no error.
    bool update_to_new_spectrum(AudioSpectrum newSpectrum);

    // Update internal data for the new specttrums.
    // Return true if there is no error.
    bool update_to_new_spectrums(const std::vector<AudioSpectrum>& new_spectrums);



private:
    std::string m_name;
    AudioTemplate m_template;

    size_t m_sample_rate;

    size_t m_numOriginalFrequencies = 0;
    size_t m_originalFreqStart = 0;
    size_t m_originalFreqEnd = 0;

    size_t m_freqStart = 0;
    size_t m_freqEnd = 0;
    // Temporal ranges of the template to match with. Used when matching a subdivided template.
    // If there is no subdivision, it will store only one pair: <0, m_template.numWindows()>
    std::vector<std::pair<size_t, size_t>> m_templateRange;
    // For each subdivided template, store its sepctrogram matrix norm
    std::vector<float> m_templateNorm;

    Mode m_mode = Mode::RAW;

    std::vector<float> m_convKernel;

    // Spectrums from audio feed. They will be matched against the template.
    std::list<AudioSpectrum> m_spectrums;
    // Norm squares of each spectrum in `m_spectrums`.
    std::list<float> m_spectrumNormSqrs;
    // How many spectrums needed to store.
    size_t m_numSpectrumsNeeded = 0;

    size_t m_lastStampTested = SIZE_MAX;
    float m_lastScale = 0.0f;
};


}
#endif
