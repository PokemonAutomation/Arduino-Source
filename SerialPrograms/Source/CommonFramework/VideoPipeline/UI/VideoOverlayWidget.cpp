/*  Video Overlay
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QPainter>
#include <QResizeEvent>
#include "VideoOverlayWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



//const ImageFloatBox ENTIRE_VIDEO(0.0, 0.0, 1.0, 1.0);



VideoOverlayWidget::~VideoOverlayWidget(){
    m_session.remove_listener(*this);
}
VideoOverlayWidget::VideoOverlayWidget(QWidget& parent, VideoOverlaySession& session)
    : QWidget(&parent)
    , m_session(session)
    , m_boxes(std::make_shared<std::vector<OverlayBox>>(session.boxes()))
    , m_texts(std::make_shared<std::vector<OverlayText>>(session.texts()))
    , m_log(std::make_shared<std::vector<OverlayLogLine>>(session.log_texts()))
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
void VideoOverlayWidget::update_log(const std::shared_ptr<const std::vector<OverlayLogLine>>& texts){
    SpinLockGuard lg(m_lock, "VideoOverlay::update_log_text()");
    m_log = texts;
}
#if 0
void VideoOverlayWidget::update_log_background(const std::shared_ptr<const std::vector<VideoOverlaySession::Box>>& bg_boxes){
    SpinLockGuard lg(m_lock, "VideoOverlay::update_log_background()");
    m_log_text_bg_boxes = bg_boxes;
}
#endif
void VideoOverlayWidget::update_stats(const std::list<OverlayStat*>* stats){
    SpinLockGuard lg(m_lock, "VideoOverlay::update_stats()");
    m_stats = stats;
}

void VideoOverlayWidget::resizeEvent(QResizeEvent* event){}
void VideoOverlayWidget::paintEvent(QPaintEvent*){
    QPainter painter(this);

    SpinLockGuard lg(m_lock, "VideoOverlay::paintEvent()");

    if (m_session.enabled_boxes()){
        update_boxes(painter);
    }
    if (m_session.enabled_text()){
        update_text(painter);
    }
    if (m_session.enabled_log()){
        update_log(painter);
    }
    if (m_session.enabled_stats() && m_stats){
        update_stats(painter);
    }
}


void VideoOverlayWidget::update_boxes(QPainter& painter){
    int width = this->width();
    int height = this->height();
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

        uint32_t red = ((uint32_t)item.color >> 16) & 0xff;
        uint32_t green = ((uint32_t)item.color >> 8) & 0xff;
        uint32_t blue = ((uint32_t)item.color >> 0) & 0xff;

        double brightness = 0.299*red + 0.587*green + 0.114*blue;
        if (brightness > 128){
            painter.setPen(QColor(0xff000000));
        }else{
            painter.setPen(QColor(0xffffffff));
        }

        painter.drawText(QPoint(xmin + padding_width, ymin - 2*padding_height), text);
    }
}
void VideoOverlayWidget::update_text(QPainter& painter){
    int width = this->width();
    int height = this->height();
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
void VideoOverlayWidget::update_log(QPainter& painter){
    if (m_log->empty()){
        return;
    }

    const double LOG_MIN_X = 0.025;
    const double LOG_WIDTH = 0.35;
    const double LOG_FONT_SIZE = 0.04;
    const double LOG_LINE_SPACING = 0.04;
    const double LOG_BORDER_X = 0.009;
    const double LOG_BORDER_Y = 0.016;
    const double LOG_MAX_Y = 0.95;

    const double log_bg_height = VideoOverlaySession::LOG_MAX_LINES * LOG_LINE_SPACING + 2*LOG_BORDER_Y;

    int width = this->width();
    int height = this->height();

    //  Draw the box.
    {
        QColor box_color(10, 10, 10, 200);
        painter.setPen(box_color);
        const int xmin = std::max((int)(width * LOG_MIN_X + 0.5), 1);
        const int ymin = std::max((int)(height * (LOG_MAX_Y - log_bg_height) + 0.5), 1);
        const int box_width = (int)(width * LOG_WIDTH + 0.5);
        const int box_height = (int)(height * log_bg_height + 0.5);
        painter.fillRect(xmin, ymin, box_width, box_height, box_color);
    }

    //  Draw the text lines.
    double x = LOG_MIN_X + LOG_BORDER_X;
    double y = LOG_MAX_Y - LOG_BORDER_Y;
    for (const OverlayLogLine& item: *m_log){
        painter.setPen(QColor((uint32_t)item.color));
        QFont text_font = this->font();
        text_font.setPointSizeF(height * LOG_FONT_SIZE);
        painter.setFont(text_font);

        const int xmin = std::max((int)(width * x + 0.5), 1);
        const int ymin = std::max((int)(height * y + 0.5), 1);

        painter.drawText(QPoint(xmin, ymin), QString::fromStdString(item.message));

        y -= LOG_LINE_SPACING;
    }
}
void VideoOverlayWidget::update_stats(QPainter& painter){
    const double TEXT_SIZE = 0.02;
    const double ROW_HEIGHT = 0.03;

    QColor box_color(10, 10, 10, 200);
    painter.setPen(box_color);

    int width = this->width();
    int height = this->height();
    int start_x = (int)(width * 0.75);

    std::vector<OverlayStatSnapshot> lines;
    for (const auto& stat : *m_stats){
        OverlayStatSnapshot snapshot = stat->get_current();
        if (!snapshot.text.empty()){
            lines.emplace_back(std::move(snapshot));
        }
    }


    painter.fillRect(
        start_x,
        0,
        width - start_x,
        (int)(height * (lines.size() * ROW_HEIGHT + 0.02)),
        box_color
    );

    size_t c = 0;
    for (const auto& stat : lines){
        painter.setPen(QColor((uint32_t)stat.color));

        QFont text_font = this->font();
        text_font.setPointSizeF(height * TEXT_SIZE);
        painter.setFont(text_font);

        int x = start_x + width * 0.01;
        int y = height * ((c + 1) * ROW_HEIGHT + 0.005);

        painter.drawText(QPoint(x, y), QString::fromStdString(stat.text));

        c++;
    }
}







}
