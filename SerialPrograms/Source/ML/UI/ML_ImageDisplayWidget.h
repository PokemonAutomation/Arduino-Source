/*  ML Image Display Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This is the Qt Widget for displaying images for ML annotation purposes.
 *  This is modified from NintendoSwitch/Framework/UI/NintendoSwitch_SwitchSystemWidget.
 */

#ifndef ML_ImageDisplayWidget_H
#define ML_ImageDisplayWidget_H

#include <QWidget>
#include "CommonFramework/VideoPipeline/UI/VideoDisplayWidget.h"

namespace PokemonAutomation{
    class CollapsibleGroupBox;
    class AudioFeed;
    class CameraSelectorWidget;
    class VideoSourceSelectorWidget;
    class VideoDisplayWidget;
    class AudioDisplayWidget;
    class AudioSelectorWidget;
    class VideoOverlay;

namespace NintendoSwitch{
    class CommandRow;
    class SwitchSystemSession;
}

namespace ML{



// UI widget for controlling and monitoring a Nintendo Switch.
// It includes:
// - A micro-controller selection UI
// - Video source selection UI
// - Audio source selection UI
// - Audio display
// - Video stream display
// It also owns a SwitchSystemSession that manages the life time of the controller,
// audio and video streams that will be exposed to automation programs.
class ImageDisplayWidget final : public QWidget, public CommandReceiver{
public:
    virtual ~ImageDisplayWidget();
    ImageDisplayWidget(
        QWidget& parent,
        NintendoSwitch::SwitchSystemSession& session,
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
    NintendoSwitch::SwitchSystemSession& m_session;

    CollapsibleGroupBox* m_group_box;

    VideoDisplayWidget* m_video_display;

    NintendoSwitch::CommandRow* m_command;

    VideoSourceSelectorWidget* m_video_selector;
};




}
}
#endif
