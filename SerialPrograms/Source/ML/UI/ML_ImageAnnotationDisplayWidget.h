/*  ML Image Annotation Display Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This is the Qt Widget for displaying images for ML annotation purposes.
 *  This is modified from NintendoSwitch/Framework/UI/NintendoSwitch_SwitchSystemWidget.
 */

#ifndef POKEMON_AUTOMATION_ML_ImageAnnotationDisplayWidget_H
#define POKEMON_AUTOMATION_ML_ImageAnnotationDisplayWidget_H

#include <QWidget>
#include "CommonFramework/VideoPipeline/UI/VideoDisplayWidget.h"
#include "CommonFramework/Globals.h"

namespace PokemonAutomation{
    class CollapsibleGroupBox;
    class AudioFeed;
    class CameraSelectorWidget;
    class VideoDisplayWidget;
    class VideoOverlay;

namespace ML{

class ImageAnnotationSourceSelectorWidget;
class ImageAnnotationDisplaySession;
class ImageAnnotationCommandRow;

// UI widget for controlling and monitoring a Nintendo Switch.
// It includes:
// - Video source selection UI
// - Video stream display
// It also owns an ImageAnnotationDisplaySession that manages the life time of the controller,
// audio and video streams that will be exposed to automation programs.
class ImageAnnotationDisplayWidget final : public QWidget, public CommandReceiver{
public:
    virtual ~ImageAnnotationDisplayWidget();
    ImageAnnotationDisplayWidget(
        QWidget& parent,
        ImageAnnotationDisplaySession& session,
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
    ImageAnnotationDisplaySession& m_session;

    CollapsibleGroupBox* m_group_box;

    VideoDisplayWidget* m_video_display;

    ImageAnnotationCommandRow* m_command;

    ImageAnnotationSourceSelectorWidget* m_selector_widget;
};




}
}
#endif
