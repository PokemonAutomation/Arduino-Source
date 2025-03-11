/*  Audio Display State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string.h>
#include <cmath>
#include <cassert>
#include <algorithm>
#include "CommonFramework/AudioPipeline/AudioConstants.h"
#include "AudioSpectrumHolder.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



AudioSpectrumHolder::AudioSpectrumHolder()
    : m_num_freqs(NUM_FFT_SAMPLES/2)
    , m_num_freq_windows(1000)
//    , m_num_freq_visualization_blocks(384)
//    , m_freq_visualization_block_boundaries(m_num_freq_visualization_blocks + 1)
//    , m_spectrograph(m_num_freq_visualization_blocks, m_num_freq_windows)
    , m_freqVisStamps(m_num_freq_windows)
{
    // We will display frequencies in log scale, so need to convert
    // log scale: 0, 1/m_numFreqVisBlocks, 2/m_numFreqVisBlocks, ..., 1.0
    // to linear scale:
    // The conversion function is: linear_value = (exp(log_value * LOG_MAX) - 1) / 10
//    const float LOG_SCALE_MAX = std::log(11.0f);

    size_t blocks = 384;
    std::vector<size_t> boundaries(blocks);

    boundaries[0] = 1;
    for (size_t i = 1; i < blocks - 1; i++){
//        const float logValue = i / (float)m_numFreqVisBlocks;
//        float linearValue = (std::exp(logValue * LOG_SCALE_MAX) - 1.f) / 10.f;
//        linearValue = std::max(std::min(linearValue, 1.0f), 0.0f);
//        boundaries[i] = std::min(size_t(linearValue * m_num_freqs + 0.5), m_num_freqs);

        //  (384 / 8 = 48) give us 48 bars per octave.
        const float x = (float)i / blocks;
        float freq = std::exp2f(8.0f * x + 3);
        size_t index = (size_t)freq;

        index = std::max(index, (size_t)1);
        index = std::min(index, m_num_freqs);
        boundaries[i] = index;
    }
    boundaries[blocks - 1] = m_num_freqs;

    //  Iterate buckets in reverse order and push the lower frequencies over so that everyone has
    //  a width of at least 1.
    size_t last = boundaries[blocks - 1];
    for (size_t c = blocks - 1; c-- > 0;){
        size_t current = boundaries[c];
        if (current >= last){
            current = last - 1;
            if (current == 0){
                current = 1;
            }
        }
        m_freq_visualization_block_boundaries.emplace_back(current);
        if (current == 1){
            break;
        }
        last = current;
    }

    blocks = m_freq_visualization_block_boundaries.size();
    std::reverse(m_freq_visualization_block_boundaries.begin(), m_freq_visualization_block_boundaries.end());


//    for (size_t i = 1; i <= m_num_freq_visualization_blocks; i++){
//        assert(m_freq_visualization_block_boundaries[i-1] < m_freq_visualization_block_boundaries[i]);
//    }
//    for (size_t i = 0; i < m_num_freq_visualization_blocks; i++){
//        cout << "index = " << m_freq_visualization_block_boundaries[i] << endl;
//    }

//    cout << "Freq vis block boundaries: ";
//    for(const auto v : m_freq_visualization_block_boundaries){
//        cout << v << " ";
//    }
//    cout << endl;

    // saveAudioFrequenciesToDisk(true);

    m_spectrograph.reset(new Spectrograph(blocks, m_num_freq_windows));
    m_last_spectrum.timestamp = current_time();
    m_last_spectrum.values.resize(blocks);
    m_last_spectrum.colors.resize(blocks);
}


void AudioSpectrumHolder::add_listener(Listener& listener){
    m_listeners.add(listener);
}
void AudioSpectrumHolder::remove_listener(Listener& listener){
    m_listeners.remove(listener);
}

void AudioSpectrumHolder::clear(){
    {
        std::lock_guard<std::mutex> lg(m_state_lock);

        m_freqVisStamps.assign(m_freqVisStamps.size(), SIZE_MAX);

        {
            // update m_spectrum_stamp_start in case the audio widget is used
            // again to store new spectrums.
            if (m_spectrums.size() > 0){
                m_spectrum_stamp_start = m_spectrums.front().stamp + 1;
            }
            m_spectrums.clear();

            m_spectrograph->clear();
            m_last_spectrum.timestamp = current_time();
            memset(m_last_spectrum.values.data(), 0, m_last_spectrum.values.size() * sizeof(float));
            memset(m_last_spectrum.colors.data(), 0, m_last_spectrum.colors.size() * sizeof(uint32_t));
        }

        m_overlay.clear();
//        cout << "AudioSpectrumHolder::clear()" << endl;
    }

    m_listeners.run_method_unique(&Listener::state_changed);
}

//void AudioSpectrumHolder::reset(){}


// TODO: move this to a common lib folder:
PA_FORCE_INLINE uint32_t jetColorMap(float v){
    if (v <= 0.f){
        return combine_rgb(0,0,0);
    }else if (v < 0.125f){
//        return qRgb(0, 0, int((0.5f + 4.f * v) * 255.f));
        return combine_rgb(0, 0, uint8_t((0.0f + 8.f * v) * 255.f));
    }else if (v < 0.375f){
        return combine_rgb(0, uint8_t((v - 0.125f)*1020.f), 255);
    }else if (v < 0.625f){
        uint8_t c = uint8_t((v - 0.375f) * 1020.f);
        return combine_rgb(c, 255, 255-c);
    }else if (v < 0.875f){
        return combine_rgb(255, 255 - uint8_t((v-0.625f) * 1020.f), 0);
    }else if (v <= 1.0){
        return combine_rgb(255 - uint8_t((v-0.875)*1020.f), 0, 0);
    }else{
        return combine_rgb(255, 255, 255);
    }
}

void AudioSpectrumHolder::push_spectrum(size_t sample_rate, std::shared_ptr<const AlignedVector<float>> fft_output){
    WallClock timestamp = current_time();

    {
        std::lock_guard<std::mutex> lg(m_state_lock);

        const AlignedVector<float>& output = *fft_output;

        {
            const size_t stamp = (m_spectrums.size() > 0) ? m_spectrums.front().stamp + 1 : m_spectrum_stamp_start;
            m_spectrums.emplace_front(stamp, sample_rate, fft_output);
            if (m_spectrums.size() > m_spectrum_history_length){
                m_spectrums.pop_back();
            }

            // std::cout << "Load FFT output , stamp " << spectrum->stamp << std::endl;
            m_freqVisStamps[m_nextFFTWindowIndex] = stamp;
        }

        //  Scale the by the square root of the transform length.
        //  For random noise input, the frequency domain will have an average
        //  magnitude of sqrt(transform length).
        float scale = std::sqrt(0.25f / (float)output.size());

//        //  Divide by output size. Since samples can never be larger than 1.0, the
//        //  frequency domain can never be larger than the FFT length. So we scale by
//        //  the FFT length to guarantee that it also stays less than 1.0.
//        float scale = 0.5f / (float)output.size();

//        float skew_factor = 999.;
//        float skew_scale = 1.f / (float)std::log1pf(skew_factor);

        // For one window, use how many blocks to show all frequencies:
        float previous = 0;
        m_last_spectrum.timestamp = timestamp;
        for (size_t i = 0; i < m_freq_visualization_block_boundaries.size() - 1; i++){
            float mag = 0.0f;
            for(size_t j = m_freq_visualization_block_boundaries[i]; j < m_freq_visualization_block_boundaries[i+1]; j++){
                mag += output[j];
            }

            size_t width = m_freq_visualization_block_boundaries[i+1] - m_freq_visualization_block_boundaries[i];

            if (width == 0){
                mag = previous;
            }else{
                mag /= width;
                mag *= scale;

                mag = std::sqrt(mag);
//                mag = std::log1pf(mag * skew_factor) * skew_scale;
//                mag = std::log1pf(std::sqrtf(mag)) * std::log1pf(1);
//                mag = std::sqrt(2*mag - mag*mag);
//                float m1 = 1 - mag;
//                mag = std::sqrtf(1 - m1*m1);

                // Clamp to [0.0, 1.0]
                mag = std::min(mag, 1.0f);
                mag = std::max(mag, 0.0f);
            }

            m_last_spectrum.values[i] = mag;
            m_last_spectrum.colors[i] = jetColorMap(mag);
            previous = mag;
        }
//        cout << "AudioSpectrumHolder::push_spectrum" << endl;
        m_spectrograph->push_spectrum(m_last_spectrum.colors.data());
        m_nextFFTWindowIndex = (m_nextFFTWindowIndex+1) % m_num_freq_windows;
//        std::cout << "Computed FFT! "  << magSum << std::endl;

        if (m_saveFreqToDisk){
            for(size_t i = 0; i < m_num_freqs; i++){
                m_freqStream << output[i] << " ";
            }
            m_freqStream << std::endl;
        }
    }
    m_listeners.run_method_unique(&Listener::state_changed);
}
void AudioSpectrumHolder::add_overlay(uint64_t starting_stamp, uint64_t end_stamp, Color color){
    {
        std::lock_guard<std::mutex> lg(m_state_lock);

        m_overlay.emplace_front(std::forward_as_tuple(starting_stamp, end_stamp, color));

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
    }
    m_listeners.run_method_unique(&Listener::state_changed);
}

std::vector<AudioSpectrum> AudioSpectrumHolder::spectrums_since(uint64_t starting_stamp){
    std::vector<AudioSpectrum> spectrums;

    std::lock_guard<std::mutex> lg(m_state_lock);

    for (const auto& ptr : m_spectrums){
        if (ptr.stamp >= starting_stamp){
            spectrums.emplace_back(ptr);
        }else{
            break;
        }
    }
    return spectrums;
}
std::vector<AudioSpectrum> AudioSpectrumHolder::spectrums_latest(size_t num_latest_spectrums){
    std::vector<AudioSpectrum> spectrums;

    std::lock_guard<std::mutex> lg(m_state_lock);

    size_t i = 0;
    for (const auto& ptr : m_spectrums){
        if (i == num_latest_spectrums){
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
    ret.image = m_spectrograph->to_image();

    //  Calculate overplay coordinates.

    //  The oldest window on the spectrogram view has the oldest timestamp,
    //  and its position is left most on the spectrogram, assigning a window ID of 0.
    size_t oldestStamp = m_freqVisStamps[m_nextFFTWindowIndex];
    size_t oldestWindowID = 0;
    //  When the audio stream starts coming in, the history of the spectrogram
    //  is not fully filled. So the oldest stamp may not be the leftmost one on the display.
    //  Here we use the validity of the time stamp to find the real oldest one.
    for (; oldestWindowID < m_num_freq_windows; oldestWindowID++){
        if (oldestStamp != SIZE_MAX){
            // it's a window with valid stamp
            break;
        }
        oldestStamp = m_freqVisStamps[(m_nextFFTWindowIndex+oldestWindowID) % m_num_freq_windows];
    }
    if (oldestStamp == SIZE_MAX){
        // we have no valid windows in the spectrogram, so no overlays to render:
        return ret;
    }
    size_t newestStamp = m_freqVisStamps[(m_nextFFTWindowIndex + m_num_freq_windows - 1) % m_num_freq_windows];
    // size_t newestWindowID = m_num_freq_windows - 1;

    for (const auto& box : m_overlay){
        const size_t starting_stamp = std::get<0>(box);
        const size_t end_stamp = std::get<1>(box);
        const Color color = std::get<2>(box);
        if (starting_stamp >= end_stamp){
            continue;
        }

        // std::cout << "Render overlay at (" << starting_stamp << ", " << end_stamp
        //     << ") oldestStamp " << oldestStamp << " wID " << oldestWindowID << " newest stamp " << newestStamp << std::endl;

        if (end_stamp <= oldestStamp || starting_stamp > newestStamp){
            continue;
        }

        ret.overlays.emplace_back(
            starting_stamp - oldestStamp + oldestWindowID,
            end_stamp - starting_stamp,
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
