/*  Switch System
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
#include "NintendoSwitch/Framework/NintendoSwitch_SwitchSystemSession.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SwitchSystemOption.h"

namespace PokemonAutomation{
    class CollapsibleGroupBox;
    class AudioFeed;
    class CameraSelectorWidget;
    class VideoDisplayWidget;
    class AudioDisplayWidget;
    class VideoOverlay;

namespace NintendoSwitch{

class CommandRow;

class SwitchSystemWidget final : public QWidget{
public:
    virtual ~SwitchSystemWidget();
    SwitchSystemWidget(
        QWidget& parent,
        SwitchSystemSession& session,
        uint64_t program_id
    );

public:
    Logger& logger();
    BotBase* botbase();
    VideoFeed& camera();
    VideoOverlay& overlay();
    AudioFeed& audio();
    void update_ui(ProgramState state);

    //  The public versions of the private QWidget key and focus event handling functions.
    //  They are needed to accept key and focus passed from CommonFramework/VideoPipeline/UI:VideoDisplayWindow.

    //  Returns false if key is not handled. (pass it up to next handler)
    bool key_press(QKeyEvent* event);
    bool key_release(QKeyEvent* event);

    void focus_in(QFocusEvent* event);
    void focus_out(QFocusEvent* event);

private:
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;
    virtual void focusInEvent(QFocusEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;

private:
    SwitchSystemSession& m_session;

    CollapsibleGroupBox* m_group_box;

    SerialPortWidget* m_serial_widget = nullptr;

    VideoDisplayWidget* m_video_display;
    AudioDisplayWidget* m_audio_display;

    CommandRow* m_command;

    CameraSelectorWidget* m_camera_widget;
    AudioSelectorWidget* m_audio_widget;
};




}
}
#endif
