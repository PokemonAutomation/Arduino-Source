

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


std::vector<float> buildSpikeKernel(size_t numFrequencies, size_t halfSampleRate){
    std::vector<float> kernel;
    // We find a good kernel when sample rate is 48K and numFrequencies is 2048:
    // [-4.f, -3.f, -2.f, -1.f, 0.f, 1.f, 2.f, 3.f, 4.f, 4.f, 3.f, 2.f, 1.f, 0.f, -1.f, -2.f, -3.f, -4.f]
    // This spans frenquency range of 17 * halfSampleRate / numFrequencies = 199.21875Hz, where 17 is the number of intervals in the above series.
    // For another sample rate and numFrequencies combination, the number of intervals is
    // 199.21875 * numFrequencies / halfSampleRate
    size_t numKernelIntervals = int(199.21875 * numFrequencies / halfSampleRate + 0.5);
    size_t slopeLen = numKernelIntervals / 2;
    for(size_t i = 0; i <= slopeLen; i++){
        kernel.push_back(-4.0f + 8.f * i / (float)slopeLen);
    }
    for(size_t i = ((numKernelIntervals+1) % 2); i <= slopeLen; i++){
        kernel.push_back(-4.0f + 8.f * (slopeLen-i)/(float)slopeLen);
    }
    return kernel;
}

// std::vector<float> buildSmoothKernel(size_t numFrequencies, size_t halfSampleRate){
//     std::vector<float> kernel;
//     // We find a good kernel when sample rate is 48K and numFrequencies is 2048:
//     // [0.0111, 0.135, 0.606, 1.0, 0.606, 0.135, 0.0111], built as Gaussian distribution with sigma(stddev) as 1.0
//     // The equation for Gaussian is exp(-x^2/(2 sigma^2))
//     // We can think sigma value as 1.0 * frequency_gap = 1.0 * halfSampleRate / numFrequencies = 11.71875 Hz
// }


SpectrogramMatcher::SpectrogramMatcher(
    std::string name,
    AudioTemplate audioTemplate, Mode mode, size_t sample_rate,
    double low_frequency_filter, size_t templateSubdivision
)
    : m_name(std::move(name))
    , m_template(std::move(audioTemplate))
    , m_sample_rate(sample_rate)
    , m_mode(mode)
{
    const size_t numTemplateWindows = m_template.numWindows();
//    cout << "numTemplateWindows = " << numTemplateWindows << endl;
    m_numOriginalFrequencies = m_template.numFrequencies();
    if (m_template.numFrequencies() == 0){  // Error case, failed to load template
        std::cout << "Error: load audio template failed" << std::endl;
        return;
//        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Unable to load audio template file.", templateFilename.toStdString());
    }

    const size_t halfSampleRate = sample_rate / 2;

    // The frequency range from [0.0, halfSampleRate / numFrequencies, 2.0 halfSampleRate / numFrequencies, ... (numFrequencies-1) halfSampleRate / numFrequencies]
    // Since human can only hear as high as 20KHz sound, matching on frequencies >= 20KHz is meaningless.
    // So the index i of the max frequency we should be matching is the one with
    // i * halfSampleRate/numFrequencies  <= 20KHz
    // i <= numFrequencies * 20K / halfSampleRate
    // We also have a cut-off threshold to remove lower frequencies. We set it to be at frequency index 5 when sample rate is 48K and numFrequencies is 2048.
    // So 5.0 24K / 2048 = 58.59375Hz. For other sample rate and numFrequencies combinations,
    // j * halfSampleRate/numFrequencies >= 58.59375 -> j >= 58.59375 * numFrequnecies / halfSampleRate

    m_originalFreqStart = int(low_frequency_filter * m_numOriginalFrequencies / halfSampleRate + 0.5);
    m_originalFreqEnd = 20000 * m_numOriginalFrequencies / halfSampleRate + 1;

    // Initialize the spike convolution kernel:
    m_convKernel = buildSpikeKernel(m_numOriginalFrequencies, halfSampleRate);

    switch(m_mode){
    case Mode::SPIKE_CONV:
    {
        // Do convolution on audio template
        const size_t numConvedFrequencies = (m_originalFreqEnd - m_originalFreqStart) - m_convKernel.size() + 1;

        AudioTemplate audio_template(numConvedFrequencies, numTemplateWindows);
        for (size_t i = 0; i < numTemplateWindows; i++){
            conv(
                m_template.getWindow(i) + m_originalFreqStart, m_originalFreqEnd - m_originalFreqStart,
                audio_template.getWindow(i)
            );
        }

        m_template = std::move(audio_template);
        m_freqStart = 0;
        m_freqEnd = numConvedFrequencies;
        break;
    }
    case Mode::AVERAGE_5:
    {
        // Avereage every 5 frequencies
        const size_t numNewFreq = (m_originalFreqEnd - m_originalFreqStart) / 5;

        AudioTemplate audio_template(numNewFreq, numTemplateWindows);
        for (size_t i = 0; i < numTemplateWindows; i++){
            for (size_t j = 0; j < numNewFreq; j++){
                const float * rawFreqMag = m_template.getWindow(i) + m_originalFreqStart + j*5;
                const float newMag = (rawFreqMag[0] + rawFreqMag[1] + rawFreqMag[2] + rawFreqMag[3] + rawFreqMag[4]) / 5.0f;
                audio_template.getWindow(i)[j] = newMag;
            }
        }

        m_template = std::move(audio_template);
        m_freqStart = 0;
        m_freqEnd = numNewFreq;
        break;
    }
    case Mode::RAW:
        m_freqStart = m_originalFreqStart;
        m_freqEnd = m_originalFreqEnd;
        break;
    }

    if (templateSubdivision <= 1){
        m_templateRange.emplace_back(0, numTemplateWindows);
        m_numSpectrumsNeeded = numTemplateWindows;
    }else{
        // Number of subdivision cannot exceed number of windows in the template.
        templateSubdivision = std::min(templateSubdivision, numTemplateWindows);
        // num windows of each subdivided template
        const size_t num_subWindows = numTemplateWindows / templateSubdivision;
        m_numSpectrumsNeeded = num_subWindows;
        for(size_t i = 0; i < templateSubdivision; i++){
            const size_t windowStart = i * num_subWindows;
            const size_t windowEnd = (i+1) * num_subWindows;
            m_templateRange.emplace_back(windowStart, windowEnd);
        }
    }
//    cout << "m_templateRange = " << m_templateRange.size() << endl;
//    cout << "m_numSpectrumsNeeded = " << m_numSpectrumsNeeded << endl;

    m_templateNorm = buildTemplateNorm();
}

uint64_t SpectrogramMatcher::latestTimestamp() const{
    if (m_spectrums.size() == 0){
        return SIZE_MAX;
    }
    return m_spectrums.front().stamp;
}

void SpectrogramMatcher::conv(const float* src, size_t num, float* dst){
//    cout << (size_t)dst % 64 << endl;

    const size_t numConvedFrequencies = num - m_convKernel.size() + 1;
    if (num < m_convKernel.size()){
        memset(dst, 0, numConvedFrequencies * sizeof(float));
        return;
    }

#if 0
    for (size_t i = 0; i < num-m_convKernel.size()+1; i++){
        dst[i] = 0.0f;
        for (size_t j = 0; j < m_convKernel.size(); j++){
            dst[i] += src[i+j] * m_convKernel[j];
        }
    }
#else
    Kernels::SpikeConvolution::compute_spike_kernel(
        dst, src, num, m_convKernel.data(), m_convKernel.size()
    );
#endif
}

std::vector<float> SpectrogramMatcher::buildTemplateNorm() const{
    std::vector<float> ret(m_templateRange.size());

    for (size_t sub_index = 0; sub_index < m_templateRange.size(); sub_index++){
        float sumSqr = 0.0f;
        for (size_t i = m_templateRange[sub_index].first; i < m_templateRange[sub_index].second; i++){
            for(size_t j = m_freqStart; j < m_freqEnd; j++){
                const float v = m_template.getWindow(i)[j];
                sumSqr += v * v;
            }
        }
        ret[sub_index] = std::sqrt(sumSqr);
    }

    return ret;
}

bool SpectrogramMatcher::update_to_new_spectrum(AudioSpectrum spectrum){
    if (m_numOriginalFrequencies != spectrum.magnitudes->size()){
        std::cout << "Error: number of frequencies don't match in SpectrogramMatcher::match() " << 
            m_numOriginalFrequencies << " " << spectrum.magnitudes->size() << std::endl;
        return false;
    }

    switch(m_mode){
    case Mode::SPIKE_CONV:
    {
        // Do the conv on new spectrum too.
        AlignedVector<float> convedSpectrum(m_template.bufferSize());
        conv(spectrum.magnitudes->data() + m_originalFreqStart,
            m_originalFreqEnd - m_originalFreqStart, convedSpectrum.data());
        
        spectrum.magnitudes = std::make_unique<AlignedVector<float>>(std::move(convedSpectrum));
        break;
    }
    case Mode::AVERAGE_5:
    {
        AlignedVector<float> avgedSpectrum(m_template.bufferSize());
        for(size_t j = 0; j < m_template.numFrequencies(); j++){
            const float * rawFreqMag = spectrum.magnitudes->data() + m_originalFreqStart + j*5;
            const float newMag = (rawFreqMag[0] + rawFreqMag[1] + rawFreqMag[2] + rawFreqMag[3] + rawFreqMag[4]) / 5.0f;
            avgedSpectrum[j] = newMag;
        }
        spectrum.magnitudes = std::make_unique<AlignedVector<float>>(std::move(avgedSpectrum));
        break;
    }
    case Mode::RAW:
        break;
    }

    // Compute the norm square (= sum squares) of the spectrum, used for matching:
    // TODO: if there will be multiple SpectrogramMatcher running on the same audio stream, can
    // move this per-spectrum computation to a shared struct for those matchers to save computation.
    float spectrumNormSqr = 0.0f;
    for (size_t i = m_freqStart; i < m_freqEnd; i++){
        float mag = (*spectrum.magnitudes)[i];
        spectrumNormSqr += mag * mag;
    }
    m_spectrumNormSqrs.push_front(spectrumNormSqr);

    m_spectrums.emplace_front(std::move(spectrum));

    return true;
}

bool SpectrogramMatcher::update_to_new_spectrums(const std::vector<AudioSpectrum>& new_spectrums){
    for (auto it = new_spectrums.rbegin(); it != new_spectrums.rend(); it++){
        if(!update_to_new_spectrum(*it)){
            return false;
        }
    }

    // pop out too old spectrums
    while (m_spectrums.size() > m_numSpectrumsNeeded){
        m_spectrums.pop_back();
        m_spectrumNormSqrs.pop_back();
    }

    return true;
}

std::pair<float, float> SpectrogramMatcher::match_sub_template(size_t sub_index) const{
#if 0
    auto iter = m_spectrums.begin();
    auto iter2 = m_spectrumNormSqrs.begin();
    float streamSumSqr = 0.0f;
    float sumMulti = 0.0f;

    const size_t template_start = m_templateRange[sub_index].first;
    const size_t template_end = m_templateRange[sub_index].second;
    for(size_t i = template_start; i < template_end; i++, iter++, iter2++){
        // match in order from latest window to oldest
        const float* templateData = m_template.getWindow(template_end-1-i);
        const float* streamData = iter->magnitudes->data();
        streamSumSqr += *iter2;
        for(size_t j = m_freqStart; j < m_freqEnd; j++){
            sumMulti += templateData[j] * streamData[j];
        }
    }
//    cout << "sumMulti = " << sumMulti << ", streamSumSqr = " << streamSumSqr << endl;
    const float scale = (streamSumSqr < 1e-6f ? 1.0f : sumMulti / streamSumSqr);

    iter = m_spectrums.begin();
    float sum = 0.0f;
    for(size_t i = template_start; i < template_end; i++, iter++){
        // match in order from latest window to oldest
        const float* templateData = m_template.getWindow(template_end-1-i);
        const float* streamData = iter->magnitudes->data();
        for(size_t j = m_freqStart; j < m_freqEnd; j++){
            float d = templateData[j] - scale * streamData[j];
            sum += d * d;
        }
    }
#else
    //  Build matrix.
    auto iter = m_spectrums.begin();
    const size_t template_start = m_templateRange[sub_index].first;
    const size_t template_end = m_templateRange[sub_index].second;
    size_t windows = template_end - template_start;
//    cout << windows << endl;
//    cout << m_spectrums.size() << " / " << windows << endl;
    size_t freqs = m_freqEnd - m_freqStart;
    std::vector<const float*> matrixA(windows);
    std::vector<const float*> matrixT(windows);
    for (size_t i = 0; i < windows; i++, iter++){
//        cout << "Template: " << ((size_t)m_template.getWindow(template_end - 1 - i) % 64) << endl;
//        cout << "Samples:  " << ((size_t)iter->magnitudes->data() % 64) << endl;
        matrixT[i] = m_freqStart + m_template.getWindow(windows - 1 - i);
        matrixA[i] = m_freqStart + iter->magnitudes->data();
//        cout << matrixT[i] << " : " << matrixA[i] << endl;
    }

    //  Compute scale.
    float scale = Kernels::ScaleInvariantMatrixMatch::compute_scale(
        freqs, windows,
        matrixA.data(), matrixT.data()
    );
    scale = std::min<float>(scale, 1000000);

    //  Compute error.
    float sum = Kernels::ScaleInvariantMatrixMatch::compute_error(
        freqs, windows,
        scale,
        matrixA.data(), matrixT.data()
    );
#endif


    float score = sqrt(sum) / m_templateNorm[0];
//    cout << "score = " << score << endl;
    score = std::min<float>(score, 1.0);

    return std::make_pair(score, scale);
}

float SpectrogramMatcher::match(const std::vector<AudioSpectrum>& new_spectrums){
    if (!update_to_new_spectrums(new_spectrums)){
        return FLT_MAX;
    }

    if (m_spectrums.size() < m_numSpectrumsNeeded){
        return FLT_MAX;
    }

    // Check whether the stored spectrums' timestamps are continuous:
    size_t curStamp = m_spectrums.front().stamp;
    size_t lastStamp = curStamp + 1;
    for (const auto& s : m_spectrums){
        if (s.stamp != lastStamp - 1){
            std::cout << "Error: SpectrogramMatcher (" + m_name + ") spectrum timestamps are not continuous:" << std::endl;

            for(const auto& sp : m_spectrums){
                std::cout << sp.stamp << ", ";
            }
            std::cout << std::endl;
            return FLT_MAX;
        }
        lastStamp--;
    }

    if (m_lastStampTested != SIZE_MAX && curStamp <= m_lastStampTested){
        return FLT_MAX;
    }
    m_lastStampTested = curStamp;
    
    // Do the match:
    float score = FLT_MAX; // the lower the score, the better the match
    if (m_templateRange.size() == 1){
        // Match the full template
        std::tie(score, m_lastScale) = match_sub_template(0);
    }else{
        // Match each individual sub-template
        for (size_t sub_template = 0; sub_template < m_templateRange.size(); sub_template++){
            float sub_template_score = FLT_MAX;
            float sub_template_scale = 1.0f;
            std::tie(sub_template_score, sub_template_scale) = match_sub_template(sub_template);
            if (sub_template_score < score){
                score = sub_template_score;
                m_lastScale = sub_template_scale;
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
    m_spectrumNormSqrs.clear();
    m_lastStampTested = SIZE_MAX;
}




}
