/*  Video Overlay
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QPainter>
#include <QResizeEvent>
#include "CommonFramework/GlobalServices.h"
#include "VideoOverlayWidget.h"

// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{



//const ImageFloatBox ENTIRE_VIDEO(0.0, 0.0, 1.0, 1.0);



void VideoOverlayWidget::detach(){
    m_session.remove_listener(*this);
    global_watchdog().remove(*this);
}
VideoOverlayWidget::~VideoOverlayWidget(){
    detach();
}
VideoOverlayWidget::VideoOverlayWidget(QWidget& parent, VideoOverlaySession& session)
    : QWidget(&parent)
    , m_session(session)
    , m_boxes(std::make_shared<std::vector<OverlayBox>>(session.boxes()))
    , m_texts(std::make_shared<std::vector<OverlayText>>(session.texts()))
    , m_images(std::make_shared<std::vector<OverlayImage>>(session.images()))
    , m_log(std::make_shared<std::vector<OverlayLogLine>>(session.log_texts()))
//    , m_stats(nullptr)
    , m_stats_paint("VideoOverlayWidget::paintEvent", "ms", 1000, std::chrono::seconds(10))
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);

//    m_boxes.insert(&ENTIRE_VIDEO);
    try{
        global_watchdog().add(*this, std::chrono::milliseconds(50));
        m_session.add_listener(*this);
    }catch (...){
        detach();
        throw;
    }
}

void VideoOverlayWidget::async_update(){
    // by using QMetaObject::invokeMethod, we can call this->update() on a non-main thread
    // without trigger Qt crash, as normally this->update() can only be called on the main
    // thread.
    QMetaObject::invokeMethod(this, [this]{ this->update(); });
}

void VideoOverlayWidget::on_overlay_update_boxes(const std::shared_ptr<const std::vector<OverlayBox>>& boxes){
    WriteSpinLock lg(m_lock, "VideoOverlay::update_boxes()");
    m_boxes = boxes;
}
void VideoOverlayWidget::on_overlay_update_text(const std::shared_ptr<const std::vector<OverlayText>>& texts){
    WriteSpinLock lg(m_lock, "VideoOverlay::update_text()");
    m_texts = texts;
}
void VideoOverlayWidget::on_overlay_update_images(const std::shared_ptr<const std::vector<OverlayImage>>& images){
    WriteSpinLock lg(m_lock, "VideoOverlay::update_images()");
    m_images = images;
}

void VideoOverlayWidget::on_overlay_update_log(const std::shared_ptr<const std::vector<OverlayLogLine>>& logs){
    WriteSpinLock lg(m_lock, "VideoOverlay::update_log_text()");
    m_log = logs;
}
#if 0
void VideoOverlayWidget::update_log_background(const std::shared_ptr<const std::vector<VideoOverlaySession::Box>>& bg_boxes){
    WriteSpinLock lg(m_lock, "VideoOverlay::update_log_background()");
    m_log_text_bg_boxes = bg_boxes;
}
#endif

void VideoOverlayWidget::on_watchdog_timeout(){
    QMetaObject::invokeMethod(this, [this]{ this->update(); });
//    static int c = 0;
//    cout << "VideoOverlayWidget::on_watchdog_timeout(): " << c++ << endl;
}


void VideoOverlayWidget::resizeEvent(QResizeEvent* event){}

void VideoOverlayWidget::paintEvent(QPaintEvent*){
    WallClock time0 = current_time();

    QPainter painter(this);
    {
        WriteSpinLock lg(m_lock, "VideoOverlay::paintEvent()");

        if (m_session.enabled_boxes()){
            render_boxes(painter);
        }
        if (m_session.enabled_text()){
            render_text(painter);
        }
        if (m_session.enabled_images()){
            render_images(painter);
        }
        if (m_session.enabled_log()){
            render_log(painter);
        }
        if (m_session.enabled_stats()){
            render_stats(painter);
        }
    }

    global_watchdog().delay(*this);

    WallClock time1 = current_time();
    uint32_t microseconds = (uint32_t)std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count();
    m_stats_paint.report_data(m_session.logger(), microseconds);
}


void VideoOverlayWidget::render_boxes(QPainter& painter){
    const int width = this->width();
    const int height = this->height();
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
void VideoOverlayWidget::render_text(QPainter& painter){
    const int width = this->width();
    const int height = this->height();
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
void VideoOverlayWidget::render_images(QPainter& painter){
    const double width = static_cast<double>(this->width());
    const double height = static_cast<double>(this->height());

    for(const auto& image_overlay: *m_images){
        QImage q_image = image_overlay.image.to_QImage_ref();
        // source rect is the entire portion of the q_image, in pixel units
        QRectF source_rect(0.0, 0.0, static_cast<double>(q_image.width()), static_cast<double>(q_image.height()));
        // build a target_rect. target_rect is what region the overlay image should appear inside the overlay viewport.
        // target_rect is in pixel units of the viewport
        const double target_start_x = width * image_overlay.box.x;
        const double target_start_y = height * image_overlay.box.y;
        const double target_width = width * image_overlay.box.width;
        const double target_height = height * image_overlay.box.height;
        QRectF target_rect(target_start_x, target_start_y, target_width, target_height);
        painter.drawImage(target_rect, q_image, source_rect);
    }
}
void VideoOverlayWidget::render_log(QPainter& painter){
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
        // set a semi-transparent dark color so that user can see the log lines while can also see
        // vaguely the video stream content behind it
        QColor box_color(10, 10, 10, 200); // r=g=b=10, alpha=200
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
void VideoOverlayWidget::render_stats(QPainter& painter){
    const double TEXT_SIZE = 0.018;
    const double ROW_HEIGHT = 0.025;

    QColor box_color(10, 10, 10, 200);
    painter.setPen(box_color);

    int width = this->width();
    int height = this->height();
    int start_x = (int)(width * 0.78);

    std::vector<OverlayStatSnapshot> lines = m_session.stats();

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
