/*  Video Display
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QResizeEvent>
#include "CommonFramework/GlobalSettingsPanel.h"
#include "VideoDisplayWidget.h"
#include "VideoDisplayWindow.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


VideoDisplayWidget::VideoDisplayWidget(
    QWidget& parent, size_t id,
    CommandReceiver& command_receiver,
    CameraSession& camera,
    VideoOverlaySession& overlay
)
    : QWidget(&parent)
    , m_id(id)
    , m_command_receiver(command_receiver)
    , m_video(camera.make_QWidget(this))
    , m_overlay(new VideoOverlayWidget(*this, overlay))
{
    m_overlay->setVisible(true);
    m_overlay->setHidden(false);
    m_overlay->raise();
    update_size(camera.current_resolution());
}




void VideoDisplayWidget::update_size(Resolution resolution){
//    static int c = 0;
//    cout << "update_size(): " << c++ << endl;
    if (m_video == nullptr){
        this->setFixedHeight(45);
        return;
    }
    int width = this->width();
    double aspect_ratio = 16. / 9;
    if (!resolution){
        CameraSession* camera = &m_video->camera();
        resolution = camera->current_resolution();
    }
//    cout << "resolution: " << resolution.width() << " x " << resolution.height() << endl;
    if (resolution){
        aspect_ratio = (double)resolution.height / resolution.width;
        int height = (int)(width * aspect_ratio);
        this->setFixedHeight(height);
    }
    m_video->setFixedSize(this->size());
    m_overlay->setFixedSize(this->size());
    m_overlay->update_size(this->size(), this->size());

//    cout << "VideoDisplayWidget: " << this->width() << " x " << this->height() << endl;
}
void VideoDisplayWidget::resizeEvent(QResizeEvent* event){
    QWidget::resizeEvent(event);
//    cout << "resizeEvent(): " << this->width() << " x " << this->height() << endl;

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


void VideoDisplayWidget::mouseDoubleClickEvent(QMouseEvent* event){
    if (!PreloadSettings::instance().DEVELOPER_MODE){
        return;
    }
    // If this widget is not already inside a VideoDisplayWindow, move it
    // into a VideoDisplayWindow
    if (dynamic_cast<VideoDisplayWindow*>(parentWidget()) == nullptr){
        move_to_new_window();
    }else{
        QWidget::mouseDoubleClickEvent(event);
    };
}

void VideoDisplayWidget::move_to_new_window(){
    // The constructor of VideoDisplayWindow handles the transfer of this VideoDisplayWidget to the new window.
    // The constructor also displays the window.
    // So there is nothing else to do in VideoDisplayWidget::move_to_new_window() besides building VideoDisplayWindow.
    new VideoDisplayWindow(this);
}

}
