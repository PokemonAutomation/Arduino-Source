/*  Audio Display Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioDisplayWidget_H
#define PokemonAutomation_AudioPipeline_AudioDisplayWidget_H

#include <QWidget>
#include "Common/Cpp/LifetimeSanitizer.h"
#include "Common/Cpp/ValueDebouncer.h"
#include "CommonFramework/AudioPipeline/AudioOption.h"
#include "CommonFramework/AudioPipeline/AudioSession.h"

namespace PokemonAutomation{

class AudioDeviceInfo;
class Logger;


class AudioDisplayWidget
    : public QWidget
    , public AudioSpectrumHolder::Listener
    , public AudioSession::StateListener
{
public:
    using AudioDisplayType = AudioOption::AudioDisplayType;

    AudioDisplayWidget(QWidget& parent, Logger& logger, AudioSession& session);
    virtual ~AudioDisplayWidget();

    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;


private:
    virtual void state_changed() override;

//    virtual void pre_input_change() override;
    virtual void post_display_change(AudioOption::AudioDisplayType display) override;


private:
    void update_size();

    void render_bars();
    void render_spectrograph();


private:
    AudioSession& m_session;
    AudioOption::AudioDisplayType m_display_type;

    int m_previous_height = 0;
    ValueDebouncer<int> m_debouncer;

    LifetimeSanitizer m_sanitizer;
};




}
#endif
