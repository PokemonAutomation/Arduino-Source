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
    class VideoDisplayWidget;
    class VideoOverlay;

namespace ML{

class ImageAnnotationSourceSelectorWidget;
class ImageAnnotationDisplaySession;
class ImageAnnotationCommandRow;

// UI widget for loading and displaying an image for image annotation for training ML models
// It includes:
// - Image file selection UI
// - Image display window
// It also owns an ImageAnnotationDisplaySession that manages the life time of the image display
// session.
// It inherits `CommandReceiver` to handle keyboard and focus events happened on the image display
// window.
class ImageAnnotationDisplayWidget final : public QWidget, public CommandReceiver{
public:
    virtual ~ImageAnnotationDisplayWidget();
    // command_receiver: if not nullptr, ImageAnnotationDisplayWidget will forward key and
    // focus events happened on the image display window to this command_receiver.
    ImageAnnotationDisplayWidget(
        QWidget& parent,
        ImageAnnotationDisplaySession& session,
        CommandReceiver* command_receiver = nullptr
    );

    // Get the folder path of the current loaded image
    std::string image_folder_path() const;

public:
    void update_ui(ProgramState state);

    //  The public versions of the private QWidget key and focus event handling functions.
    //  They are needed to accept key and focus passed from CommonFramework/VideoPipeline/UI:VideoDisplayWindow.

    //  Overwrites CommandReceiver::key_press().
    //  The public versions of the private QWidget key event handling functions.
    //  They are needed to accept key passed from CommonFramework/VideoPipeline/UI:VideoDisplayWindow.
    virtual void key_press(QKeyEvent* event) override;
    //  Overwrites CommandReceiver::key_release().
    //  The public versions of the private QWidget key event handling functions.
    //  They are needed to accept key passed from CommonFramework/VideoPipeline/UI:VideoDisplayWindow.
    virtual void key_release(QKeyEvent* event) override;

    //  Overwrites CommandReceiver::focus_in().
    //  The public versions of the private QWidget focus event handling functions.
    //  They are needed to accept focus passed from CommonFramework/VideoPipeline/UI:VideoDisplayWindow.
    virtual void focus_in(QFocusEvent* event) override;
    //  Overwrites CommandReceiver::focus_out().
    //  The public versions of the private QWidget focus event handling functions.
    //  They are needed to accept focus passed from CommonFramework/VideoPipeline/UI:VideoDisplayWindow.
    virtual void focus_out(QFocusEvent* event) override;

private:
    //  Overwrites QWidget::keyPressEvent().
    virtual void keyPressEvent(QKeyEvent* event) override;
    //  Overwrites QWidget::keyReleaseEvent().
    virtual void keyReleaseEvent(QKeyEvent* event) override;
    //  Overwrites QWidget::focusInEvent().
    virtual void focusInEvent(QFocusEvent* event) override;
    //  Overwrites QWidget::focusOutEvent().
    virtual void focusOutEvent(QFocusEvent* event) override;

private:
    ImageAnnotationDisplaySession& m_session;

    CollapsibleGroupBox* m_group_box;

    VideoDisplayWidget* m_video_display;

    ImageAnnotationCommandRow* m_command;

    ImageAnnotationSourceSelectorWidget* m_selector_widget;

    CommandReceiver* m_command_receiver;
};




}
}
#endif
