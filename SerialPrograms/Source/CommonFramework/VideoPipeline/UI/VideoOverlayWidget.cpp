/*  Video Overlay
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QPainter>
#include <QResizeEvent>
#include "VideoOverlayWidget.h"

//  REMOVE
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
    , m_boxes(std::make_shared<std::vector<OverlayBox>>(session.boxes()))
    , m_texts(std::make_shared<std::vector<OverlayText>>(session.texts()))
    , m_log_texts(std::make_shared<std::vector<OverlayText>>(session.log_texts()))
    , m_log_text_bg_boxes(std::make_shared<std::vector<VideoOverlaySession::Box>>(session.log_text_background()))
    , m_stats(nullptr)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);

//    m_boxes.insert(&ENTIRE_VIDEO);
    m_session.add_listener(*this);
}

void VideoOverlayWidget::enabled_boxes(bool enabled){
    QMetaObject::invokeMethod(this, [this]{ this->update(); });
}
void VideoOverlayWidget::enabled_text(bool enabled){
    QMetaObject::invokeMethod(this, [this]{ this->update(); });
}
void VideoOverlayWidget::enabled_log(bool enabled){
    QMetaObject::invokeMethod(this, [this]{ this->update(); });
}
void VideoOverlayWidget::enabled_stats(bool enabled){
    QMetaObject::invokeMethod(this, [this]{ this->update(); });
}

void VideoOverlayWidget::update_boxes(const std::shared_ptr<const std::vector<OverlayBox>>& boxes){
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
void VideoOverlayWidget::update_stats(const std::list<OverlayStat*>* stats){
    SpinLockGuard lg(m_lock, "VideoOverlay::update_stats()");
    m_stats = stats;
}

void VideoOverlayWidget::resizeEvent(QResizeEvent* event){}
void VideoOverlayWidget::paintEvent(QPaintEvent*){
    QPainter painter(this);

    int width = this->width();
    int height = this->height();

    SpinLockGuard lg(m_lock, "VideoOverlay::paintEvent()");

    if (m_session.enabled_boxes()){
        for (const auto& item : *m_boxes){
            QColor color = QColor((uint32_t)item.color);
            painter.setPen(color);
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


            //  Draw the label.

            if (item.label.empty()){
                continue;
            }

            QString text = QString::fromStdString(item.label);

            QFont text_font = this->font();
            text_font.setPointSizeF((int)(height * 0.015));
            painter.setFont(text_font);

            QRect br = painter.boundingRect(0, 0, width, height, 0, text);
//            cout << br.x() << " " << br.y() << " : " << br.width() << " " << br.height() << endl;

            int padding_width = (int)(width * 0.005);
            int padding_height = (int)(height * 0.004);

            int box_height = 3*padding_height + (int)(height * 0.02);
            int box_width  = 2*padding_width + br.width();

            if (ymin - box_height < 0){
                ymin += box_height;
            }

            painter.fillRect(
                xmin,
                std::max(ymin - box_height, 0),
                box_width,
                box_height,
                color
            );

            painter.setPen(QColor(0xffffffff));
            painter.drawText(QPoint(xmin + padding_width, ymin - 2*padding_height), text);

        }
    }

    if (m_session.enabled_text()){
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

    if (m_session.enabled_log()){
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

    if (m_session.enabled_stats() && m_stats){
        const double TEXT_SIZE = 0.02;
        const double ROW_HEIGHT = 0.03;

        QColor box_color(10, 10, 10, 200);
        painter.setPen(box_color);

        int start_x = (int)(width * 0.7);

        painter.fillRect(
            start_x,
            0,
            width - start_x,
            (int)(height * (m_stats->size() * ROW_HEIGHT + 0.02)),
            box_color
        );

        size_t c = 0;
        for (const auto& stat : *m_stats){
            OverlayStatSnapshot snapshot = stat->get_current();

            painter.setPen(QColor((uint32_t)snapshot.color));

            QFont text_font = this->font();
            text_font.setPointSizeF(height * TEXT_SIZE);
            painter.setFont(text_font);

            int x = start_x + width * 0.01;
            int y = height * ((c + 1) * ROW_HEIGHT + 0.005);

            painter.drawText(QPoint(x, y), QString::fromStdString(snapshot.text));

            c++;
        }
    }
}


}
