/*  Video Display
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QResizeEvent>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "Common/Cpp/PrettyPrint.h"
#include "VideoDisplayWidget.h"
#include "VideoDisplayWindow.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



WidgetStackFixedAspectRatio::WidgetStackFixedAspectRatio(QWidget& parent, SizePolicy size_policy, double aspect_ratio)
    : QWidget(&parent)
    , m_size_policy(size_policy)
    , m_aspect_ratio(aspect_ratio)
{
    m_detached_internal = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(m_detached_internal);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignCenter);
    m_stack_holder = new QWidget(m_detached_internal);
    layout->addWidget(m_stack_holder);
//    this->setFixedHeight(495);
}

void WidgetStackFixedAspectRatio::add_widget(QWidget& widget){
    widget.setParent(m_stack_holder);
    m_widgets.insert(&widget);
}
void WidgetStackFixedAspectRatio::set_size_policy(SizePolicy size_policy){
    m_size_policy = size_policy;
    clear_fixed_dimensions();
    m_debouncer.clear();
    update_size(this->size());
}
void WidgetStackFixedAspectRatio::set_aspect_ratio(double aspect_ratio){
//    cout << "WidgetStackFixedAspectRatio::set_aspect_ratio(): " << aspect_ratio << endl;
    m_aspect_ratio = aspect_ratio;
    m_debouncer.clear();
    update_size(this->size());
}

void WidgetStackFixedAspectRatio::clear_fixed_dimensions(){
    this->setMinimumSize(QSize(0, 0));
    this->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
}
void WidgetStackFixedAspectRatio::resize_to_box(QSize enclosing_box){
    int width, height;

    double box_ratio = (double)enclosing_box.width() / enclosing_box.height();
    if (m_aspect_ratio < box_ratio){
        height = enclosing_box.height();
        width = (int)(height * m_aspect_ratio);
    }else{
        width = enclosing_box.width();
        height = (int)(width / m_aspect_ratio);
    }

    QSize size(width, height);

    m_detached_internal->setFixedSize(enclosing_box);
    m_stack_holder->setFixedSize(size);
    for (QWidget* widget : m_widgets){
        widget->setFixedSize(size);
    }
}
void WidgetStackFixedAspectRatio::resize_to_width(int width){
    int previous_width = m_stack_holder->width();
//    cout << "WidgetStackFixedAspectRatio::resize_to_width(): " << width << " <- " << previous_width << endl;

    if (width == previous_width){
        return;
    }

    if (width > previous_width && width < previous_width + 50 && !m_debouncer.check(width)){
//        cout << "Supressing potential infinite resizing loop." << endl;
        return;
    }


    int height = (int)(width / m_aspect_ratio);
//    cout << "Resizing: " << width << " x " << height << " from: " << previous_width << " x " << m_stack_holder->height() << endl;

    QSize size(width, height);
    this->setFixedHeight(height);

    m_detached_internal->setFixedSize(size);
    m_stack_holder->setFixedSize(size);
    for (QWidget* widget : m_widgets){
        widget->setFixedSize(size);
    }
}
void WidgetStackFixedAspectRatio::update_size(QSize size){
    switch (m_size_policy){
    case EXPAND_TO_BOX:
        resize_to_box(size);
        return;
    case ADJUST_HEIGHT_TO_WIDTH:
        resize_to_width(size.width());
        return;
    }
}
void WidgetStackFixedAspectRatio::resizeEvent(QResizeEvent* event){
//    cout << "WidgetStackFixedAspectRatio::resizeEvent(): " << event->size().width() << " x " << event->size().height() << endl;
    update_size(event->size());
}




VideoDisplayWidget::VideoDisplayWidget(
    QWidget& parent, QLayout& holder,
    size_t id,
    CommandReceiver& command_receiver,
    CameraSession& camera,
    VideoOverlaySession& overlay
)
    : WidgetStackFixedAspectRatio(parent, WidgetStackFixedAspectRatio::ADJUST_HEIGHT_TO_WIDTH)
    , m_holder(holder)
    , m_id(id)
    , m_command_receiver(command_receiver)
    , m_overlay_session(overlay)
    , m_video(camera.make_QtWidget(this))
    , m_overlay(new VideoOverlayWidget(*this, overlay))
    , m_underlay(new QWidget(this))
    , m_source_fps(*this)
    , m_display_fps(*this)
{
    this->add_widget(*m_video);
    this->add_widget(*m_overlay);

    Resolution resolution = m_video->camera().current_resolution();
    if (resolution){
        set_aspect_ratio(resolution.aspect_ratio());
    }

    m_overlay->setVisible(true);
    m_overlay->setHidden(false);
    m_overlay->raise();

#if 1
    {
        m_underlay->setHidden(true);
        holder.addWidget(m_underlay);

        QVBoxLayout* layout = new QVBoxLayout(m_underlay);
        layout->setAlignment(Qt::AlignTop);

        QHBoxLayout* row_width = new QHBoxLayout();
        layout->addLayout(row_width);
        QHBoxLayout* row_height = new QHBoxLayout();
        layout->addLayout(row_height);

        row_width->addStretch(2);
        row_height->addStretch(2);
        row_width->addWidget(new QLabel("<b>Window Width:</b>", m_underlay), 1);
        row_height->addWidget(new QLabel("<b>Window Height:</b>", m_underlay), 1);

        m_width_box = new QLineEdit(m_underlay);
        row_width->addWidget(m_width_box, 1);
        m_height_box = new QLineEdit(m_underlay);
        row_height->addWidget(m_height_box, 1);

        row_width->addStretch(2);
        row_height->addStretch(2);

        connect(
            m_width_box, &QLineEdit::editingFinished,
            this, [this]{
                bool ok;
                int value = m_width_box->text().toInt(&ok);
                if (ok && 100 <= value){
                    m_last_width = value;
                    if (m_window){
                        m_window->resize(m_last_width, m_last_height);
                    }
                }
                m_width_box->setText(QString::number(m_last_width));
            }
        );
        connect(
            m_height_box, &QLineEdit::editingFinished,
            this, [this]{
                bool ok;
                int value = m_height_box->text().toInt(&ok);
                if (ok && 100 <= value){
                    m_last_height = value;
                    if (m_window){
                        m_window->resize(m_last_width, m_last_height);
                    }
                }
                m_height_box->setText(QString::number(m_last_height));
            }
        );
    }
#endif

    overlay.add_stat(m_source_fps);
    overlay.add_stat(m_display_fps);
}
VideoDisplayWidget::~VideoDisplayWidget(){
    //  Close the window popout first since it holds references to this class.
    move_back_from_window();
    m_overlay_session.remove_stat(m_display_fps);
    m_overlay_session.remove_stat(m_source_fps);
    delete m_underlay;
}


void VideoDisplayWidget::move_to_new_window(){
    if (m_window){
        return;
    }
    // The constructor of VideoDisplayWindow handles the transfer of this VideoDisplayWidget to the new window.
    // The constructor also displays the window.
    // So there is nothing else to do in VideoDisplayWidget::move_to_new_window() besides building VideoDisplayWindow.
    this->set_size_policy(EXPAND_TO_BOX);
    m_window.reset(new VideoDisplayWindow(this));
    m_underlay->setHidden(false);
}
void VideoDisplayWidget::move_back_from_window(){
    if (!m_window){
        return;
    }
    m_underlay->setHidden(true);
    this->set_size_policy(ADJUST_HEIGHT_TO_WIDTH);
    m_holder.addWidget(this);
//    this->resize(this->size());
//    cout << "VideoWidget Before: " << m_video->width() << " x " << m_video->height() << endl;
    m_video->resize(this->size());
//    cout << "VideoWidget After: " << m_video->width() << " x " << m_video->height() << endl;
    m_holder.update();
    m_window.reset();
}



void VideoDisplayWidget::mouseDoubleClickEvent(QMouseEvent* event){
//    if (!PreloadSettings::instance().DEVELOPER_MODE){
//        return;
//    }
    // If this widget is not already inside a VideoDisplayWindow, move it
    // into a VideoDisplayWindow
    if (!m_window){
        move_to_new_window();
    }else{
        QWidget::mouseDoubleClickEvent(event);
    }
}
void VideoDisplayWidget::paintEvent(QPaintEvent* event){
    WidgetStackFixedAspectRatio::paintEvent(event);
//    cout << "VideoDisplayWidget: " << this->width() << " x " << this->height() << endl;
//    cout << "VideoWidget: " << m_video->width() << " x " << m_video->height() << endl;
}
void VideoDisplayWidget::resizeEvent(QResizeEvent* event){
    WidgetStackFixedAspectRatio::resizeEvent(event);
    m_last_width = this->width();
    m_last_height = this->height();
    m_width_box->setText(QString::number(m_last_width));
    m_height_box->setText(QString::number(m_last_height));
}


OverlayStatSnapshot VideoSourceFPS::get_current(){
    double fps = m_parent.m_video->camera().fps_source();
    return OverlayStatSnapshot{
        "Video Source FPS: " + tostr_fixed(fps, 2),
        fps < 20 ? COLOR_RED : COLOR_WHITE
    };
}
OverlayStatSnapshot VideoDisplayFPS::get_current(){
    double fps = m_parent.m_video->camera().fps_display();
    return OverlayStatSnapshot{
        "Video Display FPS: " + (fps < 0 ? "???" : tostr_fixed(fps, 2)),
        fps >= 0 && fps < 20 ? COLOR_RED : COLOR_WHITE
    };
}



}
