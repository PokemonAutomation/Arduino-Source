

#include <cfloat>
#include <cmath>
#include <iostream>
#include <fstream>
//#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Kernels/ScaleInvariantMatrixMatch/Kernels_ScaleInvariantMatrixMatch.h"
#include "Kernels/SpikeConvolution/Kernels_SpikeConvolution.h"
#include "CommonFramework/AudioPipeline/AudioFeed.h"
#include "CommonFramework/AudioPipeline/AudioTemplate.h"
#include "SpectrogramMatcher.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


std::vector<float> build_spike_kernel(size_t num_frequencies, size_t half_sample_rate){
    std::vector<float> kernel;
    // We find a good kernel when sample rate is 48K and num_frequencies is 2048:
    // [-4.f, -3.f, -2.f, -1.f, 0.f, 1.f, 2.f, 3.f, 4.f, 4.f, 3.f, 2.f, 1.f, 0.f, -1.f, -2.f, -3.f, -4.f]
    // This spans frenquency range of 17 * half_sample_rate / num_frequencies = 199.21875Hz, where 17 is the number of intervals in the above series.
    // For another sample rate and num_frequencies combination, the number of intervals is
    // 199.21875 * num_frequencies / half_sample_rate
    size_t num_kernel_intervals = int(199.21875 * num_frequencies / half_sample_rate + 0.5);
    size_t slope_len = num_kernel_intervals / 2;
    for (size_t i = 0; i <= slope_len; i++){
        kernel.push_back(-4.0f + 8.f * i / (float)slope_len);
    }
    for (size_t i = ((num_kernel_intervals+1) % 2); i <= slope_len; i++){
        kernel.push_back(-4.0f + 8.f * (slope_len-i)/(float)slope_len);
    }
    return kernel;
}

// std::vector<float> buildSmoothKernel(size_t num_frequencies, size_t half_sample_rate){
//     std::vector<float> kernel;
//     // We find a good kernel when sample rate is 48K and num_frequencies is 2048:
//     // [0.0111, 0.135, 0.606, 1.0, 0.606, 0.135, 0.0111], built as Gaussian distribution with sigma(stddev) as 1.0
//     // The equation for Gaussian is exp(-x^2/(2 sigma^2))
//     // We can think sigma value as 1.0 * frequency_gap = 1.0 * half_sample_rate / num_frequencies = 11.71875 Hz
// }


SpectrogramMatcher::SpectrogramMatcher(
    std::string name,
    AudioTemplate audio_template, Mode mode, size_t sample_rate,
    double low_frequency_filter, size_t template_subdivision
)
    : m_name(std::move(name))
    , m_template(std::move(audio_template))
    , m_sample_rate(sample_rate)
    , m_mode(mode)
{
    const size_t num_template_windows = m_template.num_windows();
//    cout << "num_template_windows = " << num_template_windows << endl;
    m_num_original_frequencies = m_template.num_frequencies();
    if (m_template.num_frequencies() == 0){  // Error case, failed to load template
        std::cout << "Error: load audio template failed" << std::endl;
        return;
//        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Unable to load audio template file.", templateFilename.toStdString());
    }

    const size_t half_sample_rate = sample_rate / 2;

    // The frequency range from [0.0, half_sample_rate / num_frequencies, 2.0 half_sample_rate / num_frequencies, ... (num_frequencies-1) half_sample_rate / num_frequencies]
    // Since human can only hear as high as 20KHz sound, matching on frequencies >= 20KHz is meaningless.
    // So the index i of the max frequency we should be matching is the one with
    // i * half_sample_rate/num_frequencies  <= 20KHz
    // i <= num_frequencies * 20K / half_sample_rate
    // We also have a cut-off threshold to remove lower frequencies. We set it to be at frequency index 5 when sample rate is 48K and num_frequencies is 2048.
    // So 5.0 24K / 2048 = 58.59375Hz. For other sample rate and num_frequencies combinations,
    // j * half_sample_rate/num_frequencies >= 58.59375 -> j >= 58.59375 * num_frequencies / half_sample_rate

    m_original_freq_start = int(low_frequency_filter * m_num_original_frequencies / half_sample_rate + 0.5);
    m_original_freq_end = 20000 * m_num_original_frequencies / half_sample_rate + 1;

    // Initialize the spike convolution kernel:
    m_conv_kernel = build_spike_kernel(m_num_original_frequencies, half_sample_rate);

    switch(m_mode){
    case Mode::SPIKE_CONV:
    {
        // Do convolution on audio template
        const size_t num_conved_frequencies = (m_original_freq_end - m_original_freq_start) - m_conv_kernel.size() + 1;

        AudioTemplate conved_template(num_conved_frequencies, num_template_windows);
        for (size_t i = 0; i < num_template_windows; i++){
            conv(
                m_template.get_window(i) + m_original_freq_start, m_original_freq_end - m_original_freq_start,
                conved_template.get_window(i)
            );
        }

        m_template = std::move(conved_template);
        m_freq_start = 0;
        m_freq_end = num_conved_frequencies;
        break;
    }
    case Mode::AVERAGE_5:
    {
        // Avereage every 5 frequencies
        const size_t num_new_freq = (m_original_freq_end - m_original_freq_start) / 5;

        AudioTemplate avged_template(num_new_freq, num_template_windows);
        for (size_t i = 0; i < num_template_windows; i++){
            for (size_t j = 0; j < num_new_freq; j++){
                const float * raw_freq_mag = m_template.get_window(i) + m_original_freq_start + j*5;
                const float new_mag = (raw_freq_mag[0] + raw_freq_mag[1] + raw_freq_mag[2] + raw_freq_mag[3] + raw_freq_mag[4]) / 5.0f;
                avged_template.get_window(i)[j] = new_mag;
            }
        }

        m_template = std::move(avged_template);
        m_freq_start = 0;
        m_freq_end = num_new_freq;
        break;
    }
    case Mode::RAW:
        m_freq_start = m_original_freq_start;
        m_freq_end = m_original_freq_end;
        break;
    }

    if (template_subdivision <= 1){
        m_template_range.emplace_back(0, num_template_windows);
        m_num_spectrums_needed = num_template_windows;
    }else{
        // Number of subdivision cannot exceed number of windows in the template.
        template_subdivision = std::min(template_subdivision, num_template_windows);
        // num windows of each subdivided template
        const size_t num_sub_windows = num_template_windows / template_subdivision;
        m_num_spectrums_needed = num_sub_windows;
        for (size_t i = 0; i < template_subdivision; i++){
            const size_t window_start = i * num_sub_windows;
            const size_t window_end = (i+1) * num_sub_windows;
            m_template_range.emplace_back(window_start, window_end);
        }
    }
//    cout << "m_template_range = " << m_template_range.size() << endl;
//    cout << "m_num_spectrums_needed = " << m_num_spectrums_needed << endl;

    m_template_norm = build_template_norm();
}

uint64_t SpectrogramMatcher::latest_timestamp() const{
    if (m_spectrums.size() == 0){
        return SIZE_MAX;
    }
    return m_spectrums.front().stamp;
}

void SpectrogramMatcher::conv(const float* src, size_t num, float* dst){
//    cout << (size_t)dst % 64 << endl;

    const size_t num_conved_frequencies = num - m_conv_kernel.size() + 1;
    if (num < m_conv_kernel.size()){
        memset(dst, 0, num_conved_frequencies * sizeof(float));
        return;
    }

#if 0
    for (size_t i = 0; i < num-m_conv_kernel.size()+1; i++){
        dst[i] = 0.0f;
        for (size_t j = 0; j < m_conv_kernel.size(); j++){
            dst[i] += src[i+j] * m_conv_kernel[j];
        }
    }
#else
    Kernels::SpikeConvolution::compute_spike_kernel(
        dst, src, num, m_conv_kernel.data(), m_conv_kernel.size()
    );
#endif
}

std::vector<float> SpectrogramMatcher::build_template_norm() const{
    std::vector<float> ret(m_template_range.size());

    for (size_t sub_index = 0; sub_index < m_template_range.size(); sub_index++){
        float sum_sqr = 0.0f;
        for (size_t i = m_template_range[sub_index].first; i < m_template_range[sub_index].second; i++){
            for (size_t j = m_freq_start; j < m_freq_end; j++){
                const float v = m_template.get_window(i)[j];
                sum_sqr += v * v;
            }
        }
        ret[sub_index] = std::sqrt(sum_sqr);
    }

    return ret;
}

bool SpectrogramMatcher::update_to_new_spectrum(AudioSpectrum new_spectrum){
    if (m_num_original_frequencies != new_spectrum.magnitudes->size()){
        std::cout << "Error: number of frequencies don't match in SpectrogramMatcher::match() " << 
            m_num_original_frequencies << " " << new_spectrum.magnitudes->size() << std::endl;
        return false;
    }

    switch(m_mode){
    case Mode::SPIKE_CONV:
    {
        // Do the conv on new spectrum too.
        AlignedVector<float> conved_spectrum(m_template.buffer_size());
        conv(new_spectrum.magnitudes->data() + m_original_freq_start,
            m_original_freq_end - m_original_freq_start, conved_spectrum.data());
        
        new_spectrum.magnitudes = std::make_unique<AlignedVector<float>>(std::move(conved_spectrum));
        break;
    }
    case Mode::AVERAGE_5:
    {
        AlignedVector<float> avged_spectrum(m_template.buffer_size());
        for (size_t j = 0; j < m_template.num_frequencies(); j++){
            const float * raw_freq_mag = new_spectrum.magnitudes->data() + m_original_freq_start + j*5;
            const float new_mag = (raw_freq_mag[0] + raw_freq_mag[1] + raw_freq_mag[2] + raw_freq_mag[3] + raw_freq_mag[4]) / 5.0f;
            avged_spectrum[j] = new_mag;
        }
        new_spectrum.magnitudes = std::make_unique<AlignedVector<float>>(std::move(avged_spectrum));
        break;
    }
    case Mode::RAW:
        break;
    }

    // Compute the norm square (= sum squares) of the spectrum, used for matching:
    // TODO: if there will be multiple SpectrogramMatcher running on the same audio stream, can
    // move this per-spectrum computation to a shared struct for those matchers to save computation.
    float spectrum_norm_sqr = 0.0f;
    for (size_t i = m_freq_start; i < m_freq_end; i++){
        float mag = (*new_spectrum.magnitudes)[i];
        spectrum_norm_sqr += mag * mag;
    }
    m_spectrum_norm_sqrs.push_front(spectrum_norm_sqr);

    m_spectrums.emplace_front(std::move(new_spectrum));

    return true;
}

bool SpectrogramMatcher::update_to_new_spectrums(const std::vector<AudioSpectrum>& new_spectrums){
    for (auto it = new_spectrums.rbegin(); it != new_spectrums.rend(); it++){
        if(!update_to_new_spectrum(*it)){
            return false;
        }
    }

    // pop out too old spectrums
    while (m_spectrums.size() > m_num_spectrums_needed){
        m_spectrums.pop_back();
        m_spectrum_norm_sqrs.pop_back();
    }

    return true;
}

std::pair<float, float> SpectrogramMatcher::match_sub_template(size_t sub_index) const{
#if 0
    auto iter = m_spectrums.begin();
    auto iter2 = m_spectrum_norm_sqrs.begin();
    float stream_sum_sqr = 0.0f;
    float sum_multi = 0.0f;

    const size_t template_start = m_template_range[sub_index].first;
    const size_t template_end = m_template_range[sub_index].second;
    for (size_t i = template_start; i < template_end; i++, iter++, iter2++){
        // match in order from latest window to oldest
        const float* template_data = m_template.get_window(template_end-1-i);
        const float* stream_data = iter->magnitudes->data();
        stream_sum_sqr += *iter2;
        for (size_t j = m_freq_start; j < m_freq_end; j++){
            sum_multi += template_data[j] * stream_data[j];
        }
    }
//    cout << "sum_multi = " << sum_multi << ", stream_sum_sqr = " << stream_sum_sqr << endl;
    const float scale = (stream_sum_sqr < 1e-6f ? 1.0f : sum_multi / stream_sum_sqr);

    iter = m_spectrums.begin();
    float sum = 0.0f;
    for (size_t i = template_start; i < template_end; i++, iter++){
        // match in order from latest window to oldest
        const float* template_data = m_template.get_window(template_end-1-i);
        const float* stream_data = iter->magnitudes->data();
        for (size_t j = m_freq_start; j < m_freq_end; j++){
            float d = template_data[j] - scale * stream_data[j];
            sum += d * d;
        }
    }
#else
    //  Build matrix.
    auto iter = m_spectrums.begin();
    const size_t template_start = m_template_range[sub_index].first;
    const size_t template_end = m_template_range[sub_index].second;
    size_t windows = template_end - template_start;
//    cout << windows << endl;
//    cout << m_spectrums.size() << " / " << windows << endl;
    size_t freqs = m_freq_end - m_freq_start;
    std::vector<const float*> matrix_a(windows);
    std::vector<const float*> matrix_t(windows);
    for (size_t i = 0; i < windows; i++, iter++){
//        cout << "Template: " << ((size_t)m_template.get_window(template_end - 1 - i) % 64) << endl;
//        cout << "Samples:  " << ((size_t)iter->magnitudes->data() % 64) << endl;
        matrix_t[i] = m_freq_start + m_template.get_window(windows - 1 - i);
        matrix_a[i] = m_freq_start + iter->magnitudes->data();
//        cout << matrix_t[i] << " : " << matrix_a[i] << endl;
    }

    //  Compute scale.
    float scale = Kernels::ScaleInvariantMatrixMatch::compute_scale(
        freqs, windows,
        matrix_a.data(), matrix_t.data()
    );
    scale = std::min<float>(scale, 1000000);

    //  Compute error.
    float sum = Kernels::ScaleInvariantMatrixMatch::compute_error(
        freqs, windows,
        scale,
        matrix_a.data(), matrix_t.data()
    );
#endif


    float score = sqrt(sum) / m_template_norm[0];
//    cout << "score = " << score << endl;
    score = std::min<float>(score, 1.0);

    return std::make_pair(score, scale);
}

float SpectrogramMatcher::match(const std::vector<AudioSpectrum>& new_spectrums){
    if (!update_to_new_spectrums(new_spectrums)){
        return FLT_MAX;
    }

    if (m_spectrums.size() < m_num_spectrums_needed){
        return FLT_MAX;
    }

    // Check whether the stored spectrums' timestamps are continuous:
    size_t cur_stamp = m_spectrums.front().stamp;
    size_t last_stamp = cur_stamp + 1;
    for (const auto& s : m_spectrums){
        if (s.stamp != last_stamp - 1){
            std::cout << "Error: SpectrogramMatcher (" + m_name + ") spectrum timestamps are not continuous:" << std::endl;
            for (const auto& sp : m_spectrums){
                std::cout << sp.stamp << ", ";
            }
            std::cout << std::endl;
            return FLT_MAX;
        }
        last_stamp--;
    }

    if (m_last_stamp_tested != SIZE_MAX && cur_stamp <= m_last_stamp_tested){
        return FLT_MAX;
    }
    m_last_stamp_tested = cur_stamp;
    
    // Do the match:
    float score = FLT_MAX; // the lower the score, the better the match
    if (m_template_range.size() == 1){
        // Match the full template
        std::tie(score, m_last_scale) = match_sub_template(0);
    }else{
        // Match each individual sub-template
        for (size_t sub_template = 0; sub_template < m_template_range.size(); sub_template++){
            float sub_template_score = FLT_MAX;
            float sub_template_scale = 1.0f;
            std::tie(sub_template_score, sub_template_scale) = match_sub_template(sub_template);
            if (sub_template_score < score){
                score = sub_template_score;
                m_last_scale = sub_template_scale;
            }
        }
    }

    return score;
}

bool SpectrogramMatcher::skip(const std::vector<AudioSpectrum>& new_spectrums){
    // Note: ideally we don't want to have any computation while skipping.
    // But update_to_new_spectrums() may still do some filtering and vector norm computation.
    // Since the computation is relatively small and we won't be skipping lots of frames anyway,
    // this should be fine for now.
    // We can improve this later.
    return update_to_new_spectrums(new_spectrums);
}

void SpectrogramMatcher::clear(){
    m_spectrums.clear();
    m_spectrum_norm_sqrs.clear();
    m_last_stamp_tested = SIZE_MAX;
    m_last_scale = 0.0;
}




}
