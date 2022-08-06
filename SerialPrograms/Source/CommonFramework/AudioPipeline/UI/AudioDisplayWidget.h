/*  Audio Display Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioDisplayWidget_H
#define PokemonAutomation_AudioPipeline_AudioDisplayWidget_H

#include <deque>
#include <set>
#include <QWidget>
#include "Common/Cpp/LifetimeSanitizer.h"
#include "CommonFramework/AudioPipeline/AudioOption.h"
#include "CommonFramework/AudioPipeline/AudioSession.h"

namespace PokemonAutomation{

class AudioDeviceInfo;
class LoggerQt;


class AudioDisplayWidget : public QWidget, public AudioSpectrumHolder::Listener, public AudioSession::Listener{
public:
    using AudioDisplayType = AudioOption::AudioDisplayType;

    AudioDisplayWidget(QWidget& parent, Logger& logger, AudioSession& session);
    virtual ~AudioDisplayWidget();

    void close_audio();

    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;


public:
    // Below are implementation of some of functions in the `AudioFeed` interface.
    // AudioSelectWidget inherits AudioFeed, and calls the functions below to fulfill the AudioFeed interface.
    // See class `AudioFeed` for the comments of those functions.

    void add_overlay(size_t startingStamp, size_t endStamp, Color color);


private:
    virtual void state_changed() override;
    virtual void display_changed(AudioOption::AudioDisplayType display) override;

private:
    void update_size();
//    void clear();

    void render_bars();
    void render_spectrograph();

private:
    AudioSession& m_session;
    AudioOption::AudioDisplayType m_display_type;

    std::deque<int> m_width_history;
    std::set<int> m_recent_widths;

    LifetimeSanitizer m_sanitizer;
};




}
#endif
