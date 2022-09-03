/*  Video Overlay
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QPainter>
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "VideoOverlayWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



const ImageFloatBox ENTIRE_VIDEO(0.0, 0.0, 1.0, 1.0);



VideoOverlayWidget::~VideoOverlayWidget(){
    m_session.remove_listener(*this);
}
VideoOverlayWidget::VideoOverlayWidget(QWidget& parent, VideoOverlaySession& session)
    : QWidget(&parent)
    , m_session(session)
    , m_offset_x(0)
    , m_boxes(std::make_shared<std::vector<VideoOverlaySession::Box>>(session.boxes()))
    , m_texts(std::make_shared<std::vector<OverlayText>>(session.texts()))
    , m_text_bg_boxes(std::make_shared<std::vector<VideoOverlaySession::Box>>(session.boxes()))
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);

//    m_boxes.insert(&ENTIRE_VIDEO);
    m_session.add_listener(*this);
}

void VideoOverlayWidget::update_size(const QSize& widget_size, const QSize& video_size){
//    cout << "video_size(): " << video_size.width() << " x " << video_size.height() << endl;
    m_video_size = video_size;

    m_scale = (double)widget_size.height() / video_size.height();
//    cout << "widget_size = " << widget_size.width() << " x " << widget_size.height() << endl;

    int width = (int)(m_scale * video_size.width() + 0.5);
    width = std::min(width, widget_size.width());

    m_display_size = QSize(width, widget_size.height());
    m_offset_x = (widget_size.width() - width + 1) / 2;

    this->resize(widget_size);
}

void VideoOverlayWidget::box_update(const std::shared_ptr<const std::vector<VideoOverlaySession::Box>>& boxes){
    SpinLockGuard lg(m_lock, "VideoOverlay::box_update()");
    m_boxes = boxes;
}

void VideoOverlayWidget::text_update(const std::shared_ptr<const std::vector<OverlayText>>& texts){
    SpinLockGuard lg(m_lock, "VideoOverlay::text_update()");
    m_texts = texts;
}

void VideoOverlayWidget::text_background_update(const std::shared_ptr<const std::vector<VideoOverlaySession::Box>>& bg_boxes){
    SpinLockGuard lg(m_lock, "VideoOverlay::text_background_update()");
    m_text_bg_boxes = bg_boxes;
}

void VideoOverlayWidget::paintEvent(QPaintEvent*){
    QPainter painter(this);

    double width = m_scale * m_video_size.width();
    double height = m_scale * m_video_size.height();

    SpinLockGuard lg(m_lock, "VideoOverlay::paintEvent()");

    for (const auto& item : *m_text_bg_boxes){
        QColor box_color(item.color.red(), item.color.green(), item.color.blue(), item.color.alpha());
        painter.setPen(box_color);
        const int xmin = std::max((int)(width * item.box.x + 0.5), 1) + m_offset_x;
        const int ymin = std::max((int)(height * item.box.y + 0.5), 1);
        const int box_width = (int)(width * item.box.width + 0.5);
        const int box_height = (int)(height * item.box.height + 0.5);
        painter.fillRect(xmin, ymin, box_width, box_height, box_color);
    }

//    cout << "paint: " << m_boxes.size() << endl;
    for (const auto& item : *m_boxes){
        painter.setPen(QColor((uint32_t)item.color));
//        cout << box->x << " " << box->y << ", " << box->width << " x " << box->height << endl;
//        cout << (int)(width * box->x + m_offset_x + 0.5)
//             << " " << (int)(height * box->y + 0.5)
//             << ", " << (int)(width * box->width + 0.5)
//             << " x " << (int)(height * box->height + 0.5) << endl;
//        cout << painter.pen().width() << endl;

        //  Compute coordinates. Clip so that it stays in-bounds.
        int xmin = std::max((int)(width * item.box.x + 0.5), 1) + m_offset_x;
        int ymin = std::max((int)(height * item.box.y + 0.5), 1);
//        int xmax = std::min(xmin + (int)(width * box->width + 0.5), m_display_size.width() - painter.pen().width());
//        int ymax = std::min(ymin + (int)(height * box->height + 0.5), m_display_size.height() - painter.pen().width());
        int xmax = std::min(xmin + (int)(width * item.box.width + 0.5), m_display_size.width() - 1);
        int ymax = std::min(ymin + (int)(height * item.box.height + 0.5), m_display_size.height() - 1);

//        cout << "m_video_size.width() = " << m_widget_size.width() << ", xmax = " << xmax << endl;

        painter.drawRect(
            xmin,
            ymin,
            xmax - xmin,
            ymax - ymin
        );
    }

    for (const auto& item: *m_texts){
        painter.setPen(QColor((uint32_t)item.color));
        QFont text_font = this->font();
        text_font.setPointSizeF(item.font_size);
        painter.setFont(text_font);

        const int xmin = std::max((int)(width * item.x + 0.5), 1) + m_offset_x;
        const int ymin = std::max((int)(height * item.y + 0.5), 1);

        painter.drawText(QPoint(xmin, ymin), QString::fromStdString(item.message));
    }
}


}
