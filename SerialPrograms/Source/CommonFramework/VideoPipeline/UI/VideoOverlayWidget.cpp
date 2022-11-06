/*  Video Overlay
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QPainter>
#include <QResizeEvent>
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
    , m_boxes(std::make_shared<std::vector<VideoOverlaySession::Box>>(session.boxes()))
    , m_texts(std::make_shared<std::vector<OverlayText>>(session.texts()))
    , m_log_texts(std::make_shared<std::vector<OverlayText>>(session.log_texts()))
    , m_log_text_bg_boxes(std::make_shared<std::vector<VideoOverlaySession::Box>>(session.log_text_background()))
    , m_enabled_boxes(true)
    , m_enabled_text(true)
    , m_enabled_log(true)
    , m_enabled_stats(false)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);

//    m_boxes.insert(&ENTIRE_VIDEO);
    m_session.add_listener(*this);
}


void VideoOverlayWidget::update_boxes(const std::shared_ptr<const std::vector<VideoOverlaySession::Box>>& boxes){
    SpinLockGuard lg(m_lock, "VideoOverlay::update_boxes()");
    m_boxes = boxes;
}
void VideoOverlayWidget::update_text(const std::shared_ptr<const std::vector<OverlayText>>& texts){
    SpinLockGuard lg(m_lock, "VideoOverlay::update_text()");
    m_texts = texts;
}
void VideoOverlayWidget::update_log_text(const std::shared_ptr<const std::vector<OverlayText>>& texts){
    SpinLockGuard lg(m_lock, "VideoOverlay::update_log_text()");
    m_log_texts = texts;
}
void VideoOverlayWidget::update_log_background(const std::shared_ptr<const std::vector<VideoOverlaySession::Box>>& bg_boxes){
    SpinLockGuard lg(m_lock, "VideoOverlay::update_log_background()");
    m_log_text_bg_boxes = bg_boxes;
}
void VideoOverlayWidget::update_stats(const std::shared_ptr<const std::vector<OverlayStat>>& stats){
    SpinLockGuard lg(m_lock, "VideoOverlay::update_stats()");
    m_stats = stats;
}

void VideoOverlayWidget::resizeEvent(QResizeEvent* event){}
void VideoOverlayWidget::paintEvent(QPaintEvent*){
    QPainter painter(this);

    int width = this->width();
    int height = this->height();

    SpinLockGuard lg(m_lock, "VideoOverlay::paintEvent()");

    if (m_enabled_boxes){
        for (const auto& item : *m_boxes){
            painter.setPen(QColor((uint32_t)item.color));
    //        cout << box->x << " " << box->y << ", " << box->width << " x " << box->height << endl;
    //        cout << (int)(width * box->x + 0.5)
    //             << " " << (int)(height * box->y + 0.5)
    //             << ", " << (int)(width * box->width + 0.5)
    //             << " x " << (int)(height * box->height + 0.5) << endl;
    //        cout << painter.pen().width() << endl;

            //  Compute coordinates. Clip so that it stays in-bounds.
            int xmin = std::max((int)(width * item.box.x + 0.5), 1);
            int ymin = std::max((int)(height * item.box.y + 0.5), 1);
    //        int xmax = std::min(xmin + (int)(width * box->width + 0.5), width - painter.pen().width());
    //        int ymax = std::min(ymin + (int)(height * box->height + 0.5), height - painter.pen().width());
            int xmax = std::min(xmin + (int)(width * item.box.width + 0.5), width - 1);
            int ymax = std::min(ymin + (int)(height * item.box.height + 0.5), height - 1);

    //        cout << "m_video_size.width() = " << m_widget_size.width() << ", xmax = " << xmax << endl;

            painter.drawRect(
                xmin,
                ymin,
                xmax - xmin,
                ymax - ymin
            );
        }
    }

    if (m_enabled_text){
        for (const auto& item: *m_texts){
            painter.setPen(QColor((uint32_t)item.color));
            QFont text_font = this->font();
            text_font.setPointSizeF(item.font_size * height / 100.0);
            painter.setFont(text_font);

            const int xmin = std::max((int)(width * item.x + 0.5), 1);
            const int ymin = std::max((int)(height * item.y + 0.5), 1);

            painter.drawText(QPoint(xmin, ymin), QString::fromStdString(item.message));
        }
    }

    if (m_enabled_log){
        for (const auto& item : *m_log_text_bg_boxes){
            QColor box_color(item.color.red(), item.color.green(), item.color.blue(), item.color.alpha());
            painter.setPen(box_color);
            const int xmin = std::max((int)(width * item.box.x + 0.5), 1);
            const int ymin = std::max((int)(height * item.box.y + 0.5), 1);
            const int box_width = (int)(width * item.box.width + 0.5);
            const int box_height = (int)(height * item.box.height + 0.5);
            painter.fillRect(xmin, ymin, box_width, box_height, box_color);
        }

        for (const auto& item: *m_log_texts){
            painter.setPen(QColor((uint32_t)item.color));
            QFont text_font = this->font();
            text_font.setPointSizeF(item.font_size * height / 100.0);
            painter.setFont(text_font);

            const int xmin = std::max((int)(width * item.x + 0.5), 1);
            const int ymin = std::max((int)(height * item.y + 0.5), 1);

            painter.drawText(QPoint(xmin, ymin), QString::fromStdString(item.message));
        }
    }

    if (m_enabled_stats){

    }
}


}
