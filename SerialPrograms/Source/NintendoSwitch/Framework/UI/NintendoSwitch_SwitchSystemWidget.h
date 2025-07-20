/*  Switch System
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This is the Qt Widget implementation of the UI for SwitchSystemSession.
 *
 *  On construction, this class attaches itself to the session it is constructed
 *  with and automatically detaches on destruction. Therefore, this class must
 *  not outlive the session it is constructed with. While not useful, it is also
 *  safe to construct multiple UI classes attached to the same session.
 *
 *  Modifications directly to the session object will automatically update this
 *  UI class. For example, if you use Discord to change the volume of the
 *  audio playback, it will move the slider as shown by this UI.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SwitchSystemWidget_H
#define PokemonAutomation_NintendoSwitch_SwitchSystemWidget_H

#include <QWidget>
#include "CommonFramework/VideoPipeline/UI/VideoDisplayWidget.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SwitchSystemSession.h"

namespace PokemonAutomation{
    class CollapsibleGroupBox;
    class AudioFeed;
    class ControllerSelectorWidget;
    class CameraSelectorWidget;
    class VideoSourceSelectorWidget;
    class VideoDisplayWidget;
    class AudioDisplayWidget;
    class AudioSelectorWidget;
    class VideoOverlay;

namespace NintendoSwitch{

class CommandRow;

// UI widget for controlling and monitoring a Nintendo Switch.
// It includes:
// - A micro-controller selection UI
// - Video source selection UI
// - Audio source selection UI
// - Audio display
// - Video stream display
// It also owns a SwitchSystemSession that manages the life time of the controller,
// audio and video streams that will be exposed to automation programs.
class SwitchSystemWidget final : public QWidget, public CommandReceiver{
public:
    virtual ~SwitchSystemWidget();
    SwitchSystemWidget(
        QWidget& parent,
        SwitchSystemSession& session,
        uint64_t program_id
    );

public:
    void update_ui(ProgramState state);

    //  The public versions of the private QWidget key and focus event handling functions.
    //  They are needed to accept key and focus passed from CommonFramework/VideoPipeline/UI:VideoDisplayWindow.

    virtual void key_press(QKeyEvent* event) override;
    virtual void key_release(QKeyEvent* event) override;

    virtual void focus_in(QFocusEvent* event) override;
    virtual void focus_out(QFocusEvent* event) override;

private:
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;
    virtual void focusInEvent(QFocusEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;

private:
    SwitchSystemSession& m_session;

    CollapsibleGroupBox* m_group_box;

    ControllerSelectorWidget* m_controller;

    VideoDisplayWidget* m_video_display;
    AudioDisplayWidget* m_audio_display;

    CommandRow* m_command;

    VideoSourceSelectorWidget* m_video_selector;
    AudioSelectorWidget* m_audio_widget;
};




}
}
#endif
