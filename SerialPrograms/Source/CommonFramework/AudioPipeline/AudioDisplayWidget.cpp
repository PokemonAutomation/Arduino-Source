/*  Audio Display Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include <cfloat>
#include <cmath>
#include <utility>

#include <QVBoxLayout>
#include <QLabel>
#include <QLinearGradient>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QColor>
#include <QIODevice>
#include <QBuffer>
#include "Common/Compiler.h"
#include "AudioConstants.h"
#include "AudioInfo.h"
#include "AudioDisplayWidget.h"
#include "AudioThreadController.h"
#include "CommonFramework/Logging/Logger.h"
#include "Kernels/AbsFFT/Kernels_AbsFFT.h"

#include <iostream>
using std::cout;
using std::endl;

#ifdef USE_FFTREAL
#include <fftreal_wrapper.h>
#endif

namespace PokemonAutomation{


AudioDisplayWidget::AudioDisplayWidget(QWidget& parent)
     : QWidget(&parent)
     , m_numFreqs(NUM_FFT_SAMPLES/2)
     , m_numFreqWindows(500)
     , m_numFreqVisBlocks(96)
     , m_freqVisBlockBoundaries(m_numFreqVisBlocks+1)
     , m_freqVisBlocks(m_numFreqVisBlocks * m_numFreqWindows)
     , m_freqVisStamps(m_numFreqWindows)
{
    // We will display frequencies in log scale, so need to convert
    // log scale: 0, 1/m_numFreqVisBlocks, 2/m_numFreqVisBlocks, ..., 1.0
    // to linear scale:
    // The conversion function is: linear_value = (exp(log_value * LOG_MAX) - 1) / 10
//    const float LOG_SCALE_MAX = std::log(11.0f);
    
    m_freqVisBlockBoundaries[0] = 1;
    for(size_t i = 1; i < m_numFreqVisBlocks; i++){
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

AudioDisplayWidget::~AudioDisplayWidget(){ clear(); }

void AudioDisplayWidget::clear(){
    if (m_audioThreadController){
        delete m_audioThreadController;
        m_audioThreadController = nullptr;
    }

    m_freqVisBlocks.assign(m_freqVisBlocks.size(), 0.f);
    m_freqVisStamps.assign(m_freqVisStamps.size(), SIZE_MAX);
    std::lock_guard<std::mutex> lock_gd(m_spectrums_lock);
    m_spectrums.clear();
}

void AudioDisplayWidget::close_audio(){
    clear();

    update_size();
}

void AudioDisplayWidget::set_audio(
    Logger& logger,
    const AudioInfo& inputInfo,
    const QString& inputAbsoluteFilepath,
    const AudioInfo& outputInfo,
    float outputVolume
){
    clear();
    
    m_audioThreadController = new AudioThreadController(this, inputInfo, inputAbsoluteFilepath, outputInfo, outputVolume);

    update_size();
}

void AudioDisplayWidget::loadFFTOutput(const QVector<float>& fftOutput){
    // std::cout << "T" << QThread::currentThread() << " AudioDisplayWidget::loadFFTOutput() called" << std::endl;

    auto spectrum = std::make_shared<AudioSpectrum>(0, std::vector<float>(fftOutput.begin(), fftOutput.end()));
    {
        std::lock_guard<std::mutex> lock_gd(m_spectrums_lock);
        spectrum->stamp = (m_spectrums.size() > 0) ? m_spectrums.front()->stamp + 1 : 0;
        m_spectrums.push_front(spectrum);
        if (m_spectrums.size() > m_spectrum_history_length){
            m_spectrums.pop_back();
        }

        // std::cout << "Loadd FFT output , stamp " << spectrum->stamp << std::endl;
        m_freqVisStamps[m_nextFFTWindowIndex] = spectrum->stamp;
    }

    float scale = std::sqrt(0.5f / (float)fftOutput.size());
//    scale *= m_numFreqVisBlocks;
//    scale *= 100;

    // For one window, use how many blocks to show all frequencies:
    float previous = 0;
    for(size_t i = 0; i < m_numFreqVisBlocks; i++){
        float mag = 0.0f;
        for(size_t j = m_freqVisBlockBoundaries[i]; j < m_freqVisBlockBoundaries[i+1]; j++){
            mag += fftOutput[(int)j];
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

        m_freqVisBlocks[m_nextFFTWindowIndex*m_numFreqVisBlocks + i] = mag;
        previous = mag;
    }
    m_nextFFTWindowIndex = (m_nextFFTWindowIndex+1) % m_numFreqWindows;
    // std::cout << "Computed FFT! "  << magSum << std::endl;

    // Tell Qt to repaint the widget in the next drawing phase in the main loop.
    QWidget::update();

    if (m_saveFreqToDisk){
        for(size_t i = 0; i < m_numFreqs; i++){
            m_freqStream << fftOutput[(int)i] << " ";
        }
        m_freqStream << std::endl;
    }
}

// TODO: move this to a common lib folder:
QColor jetColorMap(float v){
    if (v <= 0.f){
        return QColor(0,0,0);
    }
    else if (v < 0.125f){
        return QColor(0, 0, int((0.5f + 4.f * v) * 255.f));
    }
    else if (v < 0.375f){
        return QColor(0, int((v - 0.125f)*1020.f), 255);
    }
    else if (v < 0.625f){
        int c = int((v - 0.375f) * 1020.f);
        return QColor(c, 255, 255-c);
    }
    else if (v < 0.875f){
        return QColor(255, 255 - int((v-0.625f) * 1020.f), 0);
    }
    else if (v <= 1.0){
        return QColor(255 - int((v-0.875)*1020.f), 0, 0);
    }
    else {
        return QColor(255, 255, 255);
    }
}

void AudioDisplayWidget::paintEvent(QPaintEvent* event){
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    const int widgetWidth = this->width();
    const int widgetHeight = this->height();
//    // num frequency bars
//    // -1 here because we don't show the freq-0 bar
//    const size_t numBars = m_numFreqVisBlocks-1;

    switch (m_audioDisplayType){
    case AudioDisplayType::FREQ_BARS:
    {
        const size_t barPlusGapWidth = widgetWidth / m_numFreqVisBlocks;
        const size_t barWidth = 0.8 * barPlusGapWidth;
        const size_t gapWidth = barPlusGapWidth - barWidth;
        const size_t paddingWidth = widgetWidth - m_numFreqVisBlocks * (barWidth + gapWidth);
        const size_t leftPaddingWidth = (paddingWidth + gapWidth) / 2;
        const size_t barHeight = widgetHeight - 2 * gapWidth;

        for (size_t i = 0; i < m_numFreqVisBlocks; i++){
            size_t curWindow = (m_nextFFTWindowIndex + m_numFreqWindows - 1) % m_numFreqWindows;
//            // +1 here to skip the freq-0 value
            float value = m_freqVisBlocks[curWindow * m_numFreqVisBlocks + i];
            QRect bar = rect();
            bar.setLeft((int)(rect().left() + leftPaddingWidth + (i * (gapWidth + barWidth))));
            bar.setWidth((int)barWidth);
            bar.setTop((int)(rect().top() + gapWidth + (1.0 - value) * barHeight));
            bar.setBottom((int)(rect().bottom() - gapWidth));

            painter.fillRect(bar, jetColorMap(value));
        }
        break;
    }
    case AudioDisplayType::SPECTROGRAM:
    {
        const size_t barHeight = widgetHeight / m_numFreqVisBlocks;
        const size_t barWidth = widgetWidth;
        for (size_t i = 0; i < m_numFreqVisBlocks; i++){
            QLinearGradient colorGradient(0,barHeight/2,widgetWidth, barHeight/2);
            colorGradient.setSpread(QGradient::PadSpread);
            for(size_t j = 0; j < m_numFreqWindows; j++){
                // Start with the oldest window in time:
                size_t curWindow = (m_nextFFTWindowIndex + m_numFreqWindows + j) % m_numFreqWindows;
//                // +1 here to skip the freq-0 value
                float value = m_freqVisBlocks[curWindow * m_numFreqVisBlocks + i];

                float pos = (float)j/(m_numFreqWindows - 1);
                colorGradient.setColorAt(pos, jetColorMap(value));
            }

            QRect bar = rect();
            bar.setLeft(rect().left());
            bar.setWidth((int)barWidth);
            bar.setTop((int)(rect().top() + i * barHeight));
            bar.setBottom((int)(rect().top() + (i+1) * barHeight));

            painter.fillRect(bar, QBrush(colorGradient));
        }
        
        // Now render overlays:
        
        // The oldest window on the spectrogram view has the oldest timestamp,
        // and its position is left most on the spectrogram, assigning a window ID of 0.
        size_t oldestStamp = m_freqVisStamps[m_nextFFTWindowIndex];
        size_t oldestWindowID = 0;
        // When the audio stream starts coming in, the history of the spectrogram
        // is not fully filled. So the oldest stamp may not be the leftmost one on the display.
        // Here we use the validity of the time stamp to find the real oldest one.
        for(; oldestWindowID < m_numFreqWindows; oldestWindowID++){
            if (oldestStamp != SIZE_MAX){
                // it's a window with valid stamp
                break;
            }
            oldestStamp = m_freqVisStamps[(m_nextFFTWindowIndex+oldestWindowID) % m_numFreqWindows];
        }
        if (oldestStamp == SIZE_MAX){
            // we have no valid windows in the spectrogram, so no overlays to render:
            break;
        }
        size_t newestStamp = m_freqVisStamps[(m_nextFFTWindowIndex+m_numFreqWindows-1) % m_numFreqWindows];
        // size_t newestWindowID = m_numFreqWindows - 1;

        // Each window has width: widgetWidth / (m_numFreqWindows-1) on the spectrogram
        const float FFTWindowWidth = widgetWidth / float(m_numFreqWindows-1);

        for(const auto& overlayRange: m_overlays){
            const size_t startingStamp = overlayRange.first;
            const size_t endStamp = overlayRange.second;
            if (startingStamp >= endStamp){
                continue;
            }

            // std::cout << "Render overlay at (" << startingStamp << ", " << endStamp
            //     << ") oldestStamp " << oldestStamp << " wID " << oldestWindowID << " newest stamp " << newestStamp << std::endl;

            if (endStamp <= oldestStamp || startingStamp > newestStamp){
                continue;            
            }


            int xmin = int((startingStamp - oldestStamp + oldestWindowID - 0.5) * FFTWindowWidth + 0.5);
            int ymin = rect().top();
            int rangeWidth = int(FFTWindowWidth * (endStamp - startingStamp) + 0.5);
            painter.setPen(Qt::red);
            painter.drawRect(xmin, ymin, rangeWidth, widgetHeight);
        }
        
        break;
    }
    default:
        break;
    }
}

void AudioDisplayWidget::setAudioDisplayType(AudioDisplayType type){
    m_audioDisplayType = type;
    update_size();
}

void AudioDisplayWidget::update_size(){
    int height = m_audioDisplayType == AudioDisplayType::NO_DISPLAY ? 0 : this->width() / 6;
    this->setFixedHeight(height);
}

void AudioDisplayWidget::resizeEvent(QResizeEvent* event){
    QWidget::resizeEvent(event);
    // std::cout << "Audio widget size: " << this->width() << " x " << this->height() << std::endl;

    int width = this->width();

    //  Safeguard against a resizing loop where the UI bounces between larger
    //  height with scroll bar and lower height with no scroll bar.
    auto iter = m_recent_widths.find(width);
    if (iter != m_recent_widths.end() && std::abs(width - event->oldSize().width()) < 50){
//        cout << "Supressing potential infinite resizing loop." << endl;
        return;
    }

    m_width_history.push_back(width);
    m_recent_widths.insert(width);
    if (m_width_history.size() > 10){
        m_recent_widths.erase(m_width_history[0]);
        m_width_history.pop_front();
    }

    update_size();
}


void AudioDisplayWidget::spectrums_since(size_t startingStamp, std::vector<std::shared_ptr<AudioSpectrum>>& spectrums){
    std::lock_guard<std::mutex> lock_gd(m_spectrums_lock);
    for(const auto& ptr : m_spectrums){
        if (ptr->stamp >= startingStamp){
            spectrums.push_back(ptr);
        } else{
            break;
        }
    }
}

void AudioDisplayWidget::spectrums_latest(size_t numLatestSpectrums, std::vector<std::shared_ptr<AudioSpectrum>>& spectrums){
    std::lock_guard<std::mutex> lock_gd(m_spectrums_lock);
    size_t i = 0;
    for(const auto& ptr : m_spectrums){
        if (i == numLatestSpectrums){
            break;
        }
        spectrums.push_back(ptr);
        i++;
    }
}


void AudioDisplayWidget::add_overlay(size_t startingStamp, size_t endStamp){
    m_overlays.push_front(std::make_pair(startingStamp, endStamp));

    // Now try to remove old overlays that are no longer showed on the spectrogram view.

    // get the timestamp of the oldest window in the display history.
    size_t oldest_stamp = m_freqVisStamps[m_nextFFTWindowIndex];
    // SIZE_MAX means this slot is not yet assigned an FFT window
    if (oldest_stamp != SIZE_MAX){
        while(!m_overlays.empty()){
            if (m_overlays.back().second <= oldest_stamp){
                m_overlays.pop_back();
            } else{
                break;
            }
        }
    }
}


void AudioDisplayWidget::saveAudioFrequenciesToDisk(bool enable){
    if (enable){
        if (m_saveFreqToDisk == false){
            m_saveFreqToDisk = enable;
            m_freqStream.open("./frequencies.txt");
        }
    } else if (m_saveFreqToDisk){
        m_saveFreqToDisk = enable;
        m_freqStream.close();
    }
}

}
