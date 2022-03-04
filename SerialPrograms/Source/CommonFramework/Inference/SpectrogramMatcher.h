/*  Spectrogram Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_SpectrogramMatcher_H
#define PokemonAutomation_CommonFramework_SpectrogramMatcher_H

#include <cstddef>
#include <array>
#include <memory>
#include <vector>
#include <list>
#include "CommonFramework/AudioPipeline/AudioTemplate.h"

class QString;

namespace PokemonAutomation{

class AudioSpectrum;

// Load an audio template from disk and use its spectrogram to match the
// spectrogram of the incoming audio stream.
class SpectrogramMatcher{
public:
    enum class Mode{
        // Don't do any convolution on each window of spectrum, matching raw spectrums.
        NO_CONV,
        // Do convolution on each window of spectrum with a peak detection kernel, before matching spectrums.
        SPIKE_CONV,
    };

    SpectrogramMatcher(
        AudioTemplate audioTemplate, Mode mode, size_t sampleRate,
        double lowFrequencyFilter
    );
    SpectrogramMatcher(
        const QString& templateFilename, Mode mode, size_t sampleRate,
        double lowFrequencyFilter
    );

    size_t sampleRate() const{ return m_sampleRate; }

    // Match the newest spectrums and return a match score.
    // Newer (larger timestamp) spectrums at beginning of `newSpectrums` while older (smaller
    // timestamp) spectrums at the end.
    // In invalid cases (internal error or not enough windows), return FLT_MAX
    float match(const std::vector<AudioSpectrum>& newSpectrums);

    // Pass some spectrums in but don't run match on them.
    // Used for skipping some spectrums to avoid unnecessary matching.
    // Newer (larger timestamp) spectrums at beginning of `newSpectrums` while older (smaller
    // timestamp) spectrums at the end.
    // Return true if there is no error.
    bool skip(const std::vector<AudioSpectrum>& newSpectrums);

    // Clear internal data to be used on another audio stream.
    void clear();

    size_t numTemplateWindows() const { return m_template.numWindows(); }

    // Return latest timestamp from the stored audio spectrum stream.
    // Return SIZE_MAX if there is no stored spectrum yet.
    size_t latestTimestamp() const;

    // Return the scale found by the matcher to scale the input audio stream to best
    // match the template in the last `match()`.
    // Return 0.0f if the last scale is not available.
    float lastMatchedScale() const { return m_lastScale; }

private:
    void conv(const float* src, size_t num, float* dst);
    float templateNorm() const;

    // Update internal data for the next new spectrum. Called by `updateToNewSpectrums()`.
    // Return true if there is no error.
    bool updateToNewSpectrum(AudioSpectrum newSpectrum);

    // Update internal data for the new specttrums.
    // Return true if there is no error.
    bool updateToNewSpectrums(const std::vector<AudioSpectrum>& newSpectrums);

private:
    AudioTemplate m_template;

    size_t m_sampleRate;

    size_t m_numOriginalFrequencies = 0;
    size_t m_originalFreqStart = 0;
    size_t m_originalFreqEnd = 0;

    float m_templateNorm = 0.0f;
    size_t m_freqStart = 0;
    size_t m_freqEnd = 0;

    Mode m_mode = Mode::NO_CONV;

    std::vector<float> m_spikeKernel;

    std::list<AudioSpectrum> m_spectrums;
    std::list<float> m_spectrumNormSqrs;

    size_t m_lastStampTested = SIZE_MAX;
    float m_lastScale = 0.0f;
};


}
#endif
