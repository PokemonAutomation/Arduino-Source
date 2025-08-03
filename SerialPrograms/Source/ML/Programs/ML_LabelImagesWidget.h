/*  ML Label Images Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Derived class of PanelWidget as the UI for program LabelImages.
 */

#ifndef PokemonAutomation_ML_LabelImagesWidget_H
#define PokemonAutomation_ML_LabelImagesWidget_H


#include <QKeyEvent>
#include <QFocusEvent>
#include <QWidget>
#include <chrono>
#include "Common/Cpp/Options/ConfigOption.h"
#include "CommonFramework/Panels/UI/PanelWidget.h"
#include "CommonFramework/Panels/PanelTools.h"
#include "CommonFramework/VideoPipeline/VideoSource.h"
#include "CommonFramework/VideoPipeline/UI/VideoDisplayWidget.h"
#include "CommonFramework/VideoPipeline/VideoSession.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"

class QLabel;

namespace PokemonAutomation{
namespace ML{


class LabelImages;
class ImageAnnotationDisplayWidget;
class ImageAnnotationDisplaySession;


class LabelImages_Widget : public PanelWidget,
                           public ConfigOption::Listener,
                           public VideoSession::StateListener,
                           public CommandReceiver,
                           public VideoOverlay::MouseListener{
public:
    ~LabelImages_Widget();
    LabelImages_Widget(
        QWidget& parent,
        LabelImages& instance,
        PanelHolder& holder
    );

    //  Overwrites ConfigOption::Listener::on_config_value_changed().
    virtual void on_config_value_changed(void* object) override;

    //  Overwrites VideoSession::StateListener::post_startup().
    virtual void post_startup(VideoSource* source) override;

    //  Overwrites CommandReceiver::key_press().
    virtual void key_press(QKeyEvent* event) override;
    //  Overwrites CommandReceiver::key_release().
    virtual void key_release(QKeyEvent* event) override;
    //  Overwrites CommandReceiver::focus_in().
    virtual void focus_in(QFocusEvent* event) override;
    //  Overwrites CommandReceiver::focus_out().
    virtual void focus_out(QFocusEvent* event) override;

    //  Overwrites VideoOverlay::MouseListener::on_mouse_press().
    virtual void on_mouse_press(double x, double y) override;
    //  Overwrites VideoOverlay::MouseListener::on_mouse_release().
    virtual void on_mouse_release(double x, double y) override;
    //  Overwrites VideoOverlay::MouseListener::on_mouse_move().
    virtual void on_mouse_move(double x, double y) override;

private:
    LabelImages& m_program;
    ImageAnnotationDisplaySession& m_display_session;

    ImageAnnotationDisplayWidget* m_image_display_widget;
    
    // show the info about the loaded image embedding data corresponding to the currently
    // displayed image
    QLabel* m_embedding_info_label = nullptr;

    // TODO: see if we can use
    // Common/Cpp/Options/BoxFloatOption.h and Common/Qt/Options/BoxFloatWidget.h as UI options

    std::optional<std::pair<double, double>> m_mouse_start;
    std::optional<std::pair<double, double>> m_mouse_end;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_mouse_start_time;

    bool m_shift_pressed = false;
    bool m_control_pressed = false;
    double m_cur_mouse_x = 0.0;
    double m_cur_mouse_y = 0.0;
};


}
}
#endif