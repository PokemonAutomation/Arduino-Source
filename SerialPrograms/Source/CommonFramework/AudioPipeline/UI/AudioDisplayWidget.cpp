/*  Audio Display Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include <QPaintEvent>
#include <QPainter>
#include "Common/Qt/Redispatch.h"
#include "AudioDisplayWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{




AudioDisplayWidget::AudioDisplayWidget(QWidget& parent, Logger& logger, AudioSession& session)
    : QWidget(&parent)
    , m_session(session)
    , m_display_type(session.display_type())
{
    m_session.add_state_listener(*this);
    m_session.add_spectrum_listener(*this);
}

AudioDisplayWidget::~AudioDisplayWidget(){
    m_session.remove_spectrum_listener(*this);
    m_session.remove_state_listener(*this);
}



void AudioDisplayWidget::state_changed(){
//    cout << "AudioDisplayWidget::state_changed()" << endl;
    QMetaObject::invokeMethod(
        this, [this]{
            auto scope_check = m_sanitizer.check_scope();
            update_size();
            QWidget::update();
        }, Qt::QueuedConnection
    );
}
#if 0
void AudioDisplayWidget::pre_input_change(){
    QMetaObject::invokeMethod(
        this, [this]{
            auto scope_check = m_sanitizer.check_scope();
            update_size();
            QWidget::update();
        }, Qt::QueuedConnection
    );
}
#endif
void AudioDisplayWidget::post_display_change(AudioOption::AudioDisplayType display){
//    cout << "AudioDisplayWidget::display_changed()" << endl;
    QMetaObject::invokeMethod(
        this, [this, display]{
            auto scope_check = m_sanitizer.check_scope();
            m_display_type = display;
            update_size();
            QWidget::update();
        }, Qt::QueuedConnection
    );
}



void AudioDisplayWidget::render_bars(){
//    cout << "AudioDisplayWidget::render_bars()" << endl;

    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    const int widgetWidth = this->width();
    const int widgetHeight = this->height();

    AudioSpectrumHolder::SpectrumSnapshot last_spectrum = m_session.spectrums().get_last_spectrum();

    //  Don't render if it's too old.
    if (last_spectrum.timestamp < current_time() - std::chrono::milliseconds(500)){
        return;
    }

    size_t num_buckets = last_spectrum.values.size();

#if 0
    for (int c = 0; c < 10; c++){
        cout << last_spectrum.values[c] << ", ";
    }
    cout << endl;
#endif

    const size_t barPlusGapWidth = widgetWidth / num_buckets;
    const size_t barWidth = 0.8 * barPlusGapWidth;
    const size_t gapWidth = barPlusGapWidth - barWidth;
    const size_t paddingWidth = widgetWidth - num_buckets * (barWidth + gapWidth);
    const size_t leftPaddingWidth = (paddingWidth + gapWidth) / 2;
    const size_t barHeight = widgetHeight - 2 * gapWidth;
//    cout << "barHeight = " << barHeight << endl;

    double bucket_rcp = (double)widgetWidth / num_buckets;

    for (size_t i = 0; i < num_buckets; i++){
        int s = (int)((i + 0  ) * bucket_rcp);    //  Start of bucket.
        int e = (int)((i + 0.8) * bucket_rcp);    //  End of bucket.

//        size_t curWindow = (m_nextFFTWindowIndex + m_num_freq_windows - 1) % m_num_freq_windows;
//            // +1 here to skip the freq-0 value
//        float value = m_freqVisBlocks[curWindow * m_numFreqVisBlocks + i];
        float value = last_spectrum.values[i];
        QRect bar = rect();
        bar.setLeft((int)(rect().left() + leftPaddingWidth + (i * (gapWidth + barWidth))));
        bar.setWidth((int)barWidth);
        bar.setTop((int)(rect().top() + gapWidth + (1.0 - value) * barHeight));
        bar.setBottom((int)(rect().bottom() - gapWidth));

        bar.setLeft(s);
        bar.setWidth(e - s);

        painter.fillRect(bar, last_spectrum.colors[i]);
    }
}
void AudioDisplayWidget::render_spectrograph(){
    QPainter painter(this);

    const int widgetWidth = this->width();
    const int widgetHeight = this->height();

    AudioSpectrumHolder::SpectrographSnapshot snapshot = m_session.spectrums().get_spectrograph();
    {
        QImage graph_image = snapshot.image.to_QImage_ref();
        graph_image = graph_image.scaled(
            widgetWidth, widgetHeight,
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        );
        painter.fillRect(rect(), graph_image);
    }

    // Now render overlays:

    // Each window has width: widgetWidth / (m_num_freq_windows-1) on the spectrogram
    const float FFTWindowWidth = widgetWidth / float(snapshot.image.width() - 1);

    for (const auto& box : snapshot.overlays){
        int xmin = int(std::get<0>(box) * FFTWindowWidth + 0.5);
        int ymin = rect().top();
        int rangeWidth = int(FFTWindowWidth * std::get<1>(box) + 0.5);
        painter.setPen(QColor((uint32_t)std::get<2>(box)));
        painter.drawRect(xmin, ymin + 1, rangeWidth, widgetHeight - 2);
    }
}
void AudioDisplayWidget::paintEvent(QPaintEvent* event){
//    cout << "AudioDisplayWidget::paintEvent()" << endl;
    QWidget::paintEvent(event);

    switch (m_display_type){
    case AudioDisplayType::FREQ_BARS:
        render_bars();
        break;
    case AudioDisplayType::SPECTROGRAM:
        render_spectrograph();
        break;
    default:
        break;
    }
}


void AudioDisplayWidget::update_size(){
    int height = m_display_type == AudioDisplayType::NO_DISPLAY ? 0 : this->width() / 6;
//    cout << "AudioDisplayWidget::update_size(): " << height << " <- " << m_previous_height << endl;

    if (height == m_previous_height){
        return;
    }

    if (height > m_previous_height && height < m_previous_height + 10 && !m_debouncer.check(height)){
//        cout << "Suppressing potential infinite resizing loop." << endl;
        return;
    }

    this->setFixedHeight(height);
    m_previous_height = height;
}

void AudioDisplayWidget::resizeEvent(QResizeEvent* event){
    QWidget::resizeEvent(event);

    update_size();
}




}
