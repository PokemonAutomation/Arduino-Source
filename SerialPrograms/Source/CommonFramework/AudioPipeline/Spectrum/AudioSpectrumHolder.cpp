/*  Audio Display State
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <string.h>
#include <cmath>
#include <cassert>
#include "CommonFramework/AudioPipeline/AudioConstants.h"
#include "AudioSpectrumHolder.h"

namespace PokemonAutomation{



AudioSpectrumHolder::AudioSpectrumHolder()
    : m_numFreqs(NUM_FFT_SAMPLES/2)
    , m_numFreqWindows(1000)
    , m_numFreqVisBlocks(96)
    , m_freqVisBlockBoundaries(m_numFreqVisBlocks+1)
    , m_spectrograph(m_numFreqVisBlocks, m_numFreqWindows)
    , m_freqVisStamps(m_numFreqWindows)
{
    m_last_spectrum.values.resize(m_numFreqVisBlocks);
    m_last_spectrum.colors.resize(m_numFreqVisBlocks);

    // We will display frequencies in log scale, so need to convert
    // log scale: 0, 1/m_numFreqVisBlocks, 2/m_numFreqVisBlocks, ..., 1.0
    // to linear scale:
    // The conversion function is: linear_value = (exp(log_value * LOG_MAX) - 1) / 10
//    const float LOG_SCALE_MAX = std::log(11.0f);

    m_freqVisBlockBoundaries[0] = 1;
    for (size_t i = 1; i < m_numFreqVisBlocks; i++){
//        const float logValue = i / (float)m_numFreqVisBlocks;
//        float linearValue = (std::exp(logValue * LOG_SCALE_MAX) - 1.f) / 10.f;
//        linearValue = std::max(std::min(linearValue, 1.0f), 0.0f);
//        m_freqVisBlockBoundaries[i] = std::min(size_t(linearValue * m_numFreqs + 0.5), m_numFreqs);

        //  (96 / 8 = 12) give us 12 bars per octave.
        const float x = (float)i / m_numFreqVisBlocks;
        float freq = std::exp2f(8.0f * x + 3);
        size_t index = (size_t)freq;

        index = std::max(index, (size_t)1);
        index = std::min(index, m_numFreqs);
        m_freqVisBlockBoundaries[i] = index;
    }
    m_freqVisBlockBoundaries[m_numFreqVisBlocks] = m_numFreqs;

    //  Iterate buckets in reverse order and push the lower frequencies over so that everyone has
    //  a width of at least 1.
    size_t last = m_freqVisBlockBoundaries[m_numFreqVisBlocks - 1];
    for (size_t c = m_numFreqVisBlocks - 1; c-- > 0;){
        size_t current = m_freqVisBlockBoundaries[c];
        if (current >= last){
            current = last - 1;
            if (current == 0){
                current = 1;
            }
            m_freqVisBlockBoundaries[c] = current;
        }
        last = current;
    }

    for (size_t i = 1; i <= m_numFreqVisBlocks; i++){
        assert(m_freqVisBlockBoundaries[i-1] < m_freqVisBlockBoundaries[i]);
    }
    for (size_t i = 0; i < m_numFreqVisBlocks; i++){
//        cout << "index = " << m_freqVisBlockBoundaries[i] << endl;
    }

    // std::cout << "Freq vis block boundaries: ";
    // for(const auto v : m_freqVisBlockBoundaries){
    //     std::cout << v << " ";
    // }
    // std::cout << std::endl;

    // saveAudioFrequenciesToDisk(true);
}


void AudioSpectrumHolder::add_listener(Listener& listener){
    std::lock_guard<std::mutex> lg(m_state_lock);
    m_listeners.insert(&listener);
}
void AudioSpectrumHolder::remove_listener(Listener& listener){
    std::lock_guard<std::mutex> lg(m_state_lock);
    m_listeners.erase(&listener);
}

void AudioSpectrumHolder::clear(){
    std::lock_guard<std::mutex> lg(m_state_lock);

    m_freqVisStamps.assign(m_freqVisStamps.size(), SIZE_MAX);

    {
        // update m_spectrum_stamp_start in case the audio widget is used
        // again to store new spectrums.
        if (m_spectrums.size() > 0){
            m_spectrum_stamp_start = m_spectrums.front().stamp + 1;
        }
        m_spectrums.clear();

        m_spectrograph.clear();
        memset(m_last_spectrum.values.data(), 0, m_last_spectrum.values.size() * sizeof(float));
        memset(m_last_spectrum.colors.data(), 0, m_last_spectrum.colors.size() * sizeof(uint32_t));
    }

    m_overlay.clear();

    for (Listener* listener : m_listeners){
        listener->audio_cleared();
    }
}

//void AudioSpectrumHolder::reset(){}


// TODO: move this to a common lib folder:
PA_FORCE_INLINE uint32_t jetColorMap(float v){
    if (v <= 0.f){
        return combine_rgb(0,0,0);
    }
    else if (v < 0.125f){
//        return qRgb(0, 0, int((0.5f + 4.f * v) * 255.f));
        return combine_rgb(0, 0, uint8_t((0.0f + 8.f * v) * 255.f));
    }
    else if (v < 0.375f){
        return combine_rgb(0, uint8_t((v - 0.125f)*1020.f), 255);
    }
    else if (v < 0.625f){
        uint8_t c = uint8_t((v - 0.375f) * 1020.f);
        return combine_rgb(c, 255, 255-c);
    }
    else if (v < 0.875f){
        return combine_rgb(255, 255 - uint8_t((v-0.625f) * 1020.f), 0);
    }
    else if (v <= 1.0){
        return combine_rgb(255 - uint8_t((v-0.875)*1020.f), 0, 0);
    }
    else {
        return combine_rgb(255, 255, 255);
    }
}

void AudioSpectrumHolder::push_spectrum(size_t sampleRate, std::shared_ptr<const AlignedVector<float>> fftOutput){
    std::lock_guard<std::mutex> lg(m_state_lock);

    const AlignedVector<float>& output = *fftOutput;

    {
        const size_t stamp = (m_spectrums.size() > 0) ? m_spectrums.front().stamp + 1 : m_spectrum_stamp_start;
        m_spectrums.emplace_front(stamp, sampleRate, fftOutput);
        if (m_spectrums.size() > m_spectrum_history_length){
            m_spectrums.pop_back();
        }

        // std::cout << "Loadd FFT output , stamp " << spectrum->stamp << std::endl;
        m_freqVisStamps[m_nextFFTWindowIndex] = stamp;
    }

    float scale = std::sqrt(0.5f / (float)output.size());
//    scale *= m_numFreqVisBlocks;
//    scale *= 100;

    // For one window, use how many blocks to show all frequencies:
    float previous = 0;
    for (size_t i = 0; i < m_numFreqVisBlocks; i++){
        float mag = 0.0f;
        for(size_t j = m_freqVisBlockBoundaries[i]; j < m_freqVisBlockBoundaries[i+1]; j++){
            mag += output[j];
        }

        size_t width = m_freqVisBlockBoundaries[i+1] - m_freqVisBlockBoundaries[i];

        if (width == 0){
            mag = previous;
        }else{
            mag /= width;
            mag *= scale;

            mag = std::sqrt(mag);
#if 0
//            cout << mag << endl;
//            mag = std::log10(mag * 1000 + 1);
//            mag /= 3;

            mag = std::log(mag * 10.0f + 1.0f);
            // TODO: may need to scale based on game audio volume setting
            // Assuming the max freq magnitude we can get is 20.0, so
            // log(20 * 10 + 1.0) = log(201)
            const float max_log = std::log(201.f);
            mag /= max_log;
#endif

            // Clamp to [0.0, 1.0]
            mag = std::min(mag, 1.0f);
            mag = std::max(mag, 0.0f);
        }

        m_last_spectrum.values[i] = mag;
        m_last_spectrum.colors[i] = jetColorMap(mag);
        previous = mag;
    }
    m_spectrograph.push_spectrum(m_last_spectrum.colors.data());
    m_nextFFTWindowIndex = (m_nextFFTWindowIndex+1) % m_numFreqWindows;
    // std::cout << "Computed FFT! "  << magSum << std::endl;

    if (m_saveFreqToDisk){
        for(size_t i = 0; i < m_numFreqs; i++){
            m_freqStream << output[i] << " ";
        }
        m_freqStream << std::endl;
    }

    for (Listener* listener : m_listeners){
        listener->on_new_spectrum();
    }
}
void AudioSpectrumHolder::add_overlay(uint64_t startingStamp, uint64_t endStamp, Color color){
    std::lock_guard<std::mutex> lg(m_state_lock);

    m_overlay.emplace_front(std::forward_as_tuple(startingStamp, endStamp, color));

    // Now try to remove old overlays that are no longer showed on the spectrogram view.

    // get the timestamp of the oldest window in the display history.
    uint64_t oldestStamp = m_freqVisStamps[m_nextFFTWindowIndex];
    // SIZE_MAX means this slot is not yet assigned an FFT window
    if (oldestStamp != SIZE_MAX){
        // Note: in this file we never consider the case that stamp may overflow.
        // It requires on the order of 1e10 years to overflow if we have about 25ms per stamp.
        while(!m_overlay.empty() && std::get<1>(m_overlay.back()) <= oldestStamp){
            m_overlay.pop_back();
        }
    }

    for (Listener* listener : m_listeners){
        listener->on_new_spectrum();
    }
}

std::vector<AudioSpectrum> AudioSpectrumHolder::spectrums_since(size_t startingStamp){
    std::vector<AudioSpectrum> spectrums;

    std::lock_guard<std::mutex> lg(m_state_lock);

    for (const auto& ptr : m_spectrums){
        if (ptr.stamp >= startingStamp){
            spectrums.emplace_back(ptr);
        } else{
            break;
        }
    }
    return spectrums;
}
std::vector<AudioSpectrum> AudioSpectrumHolder::spectrums_latest(size_t numLatestSpectrums){
    std::vector<AudioSpectrum> spectrums;

    std::lock_guard<std::mutex> lg(m_state_lock);

    size_t i = 0;
    for (const auto& ptr : m_spectrums){
        if (i == numLatestSpectrums){
            break;
        }
        spectrums.push_back(ptr);
        i++;
    }
    return spectrums;
}
AudioSpectrumHolder::SpectrumSnapshot AudioSpectrumHolder::get_last_spectrum() const{
    std::lock_guard<std::mutex> lg(m_state_lock);
    return m_last_spectrum;
}
AudioSpectrumHolder::SpectrographSnapshot AudioSpectrumHolder::get_spectrograph() const{
    std::lock_guard<std::mutex> lg(m_state_lock);

    SpectrographSnapshot ret;
    ret.image = m_spectrograph.to_image();

    //  Calculate overplay coordinates.

    //  The oldest window on the spectrogram view has the oldest timestamp,
    //  and its position is left most on the spectrogram, assigning a window ID of 0.
    size_t oldestStamp = m_freqVisStamps[m_nextFFTWindowIndex];
    size_t oldestWindowID = 0;
    //  When the audio stream starts coming in, the history of the spectrogram
    //  is not fully filled. So the oldest stamp may not be the leftmost one on the display.
    //  Here we use the validity of the time stamp to find the real oldest one.
    for (; oldestWindowID < m_numFreqWindows; oldestWindowID++){
        if (oldestStamp != SIZE_MAX){
            // it's a window with valid stamp
            break;
        }
        oldestStamp = m_freqVisStamps[(m_nextFFTWindowIndex+oldestWindowID) % m_numFreqWindows];
    }
    if (oldestStamp == SIZE_MAX){
        // we have no valid windows in the spectrogram, so no overlays to render:
        return ret;
    }
    size_t newestStamp = m_freqVisStamps[(m_nextFFTWindowIndex + m_numFreqWindows - 1) % m_numFreqWindows];
    // size_t newestWindowID = m_numFreqWindows - 1;

    for (const auto& box : m_overlay){
        const size_t startingStamp = std::get<0>(box);
        const size_t endStamp = std::get<1>(box);
        const Color color = std::get<2>(box);
        if (startingStamp >= endStamp){
            continue;
        }

        // std::cout << "Render overlay at (" << startingStamp << ", " << endStamp
        //     << ") oldestStamp " << oldestStamp << " wID " << oldestWindowID << " newest stamp " << newestStamp << std::endl;

        if (endStamp <= oldestStamp || startingStamp > newestStamp){
            continue;
        }

        ret.overlays.emplace_back(
            startingStamp - oldestStamp + oldestWindowID,
            endStamp - startingStamp,
            color
        );
    }
    return ret;
}


void AudioSpectrumHolder::saveAudioFrequenciesToDisk(bool enable){
    std::lock_guard<std::mutex> lg(m_state_lock);
    if (enable){
        if (m_saveFreqToDisk == false){
            m_saveFreqToDisk = enable;
            m_freqStream.open("./frequencies.txt");
        }
    }else if (m_saveFreqToDisk){
        m_saveFreqToDisk = enable;
        m_freqStream.close();
    }
}




}
