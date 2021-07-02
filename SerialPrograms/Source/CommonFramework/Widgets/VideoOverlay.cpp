/*  Video Overlay
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QPainter>
#include "VideoOverlay.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



const InferenceBox ENTIRE_VIDEO(Qt::red, 0.0, 0.0, 1.0, 1.0);




VideoOverlay::VideoOverlay(QWidget& parent)
    : QWidget(&parent)
    , m_offset_x(0)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);

//    m_boxes.insert(&ENTIRE_VIDEO);
}

void VideoOverlay::operator+=(const InferenceBox& box){
    SpinLockGuard lg(m_lock, "VideoOverlay::operator+=()");
    m_boxes.insert(&box);
}
void VideoOverlay::operator-=(const InferenceBox& box){
    SpinLockGuard lg(m_lock, "VideoOverlay::operator-=()");
    m_boxes.erase(&box);
}

void VideoOverlay::update_size(const QSize& widget_size, const QSize& video_size){
//    cout << "update_size(): " << video_size.width() << " x " << video_size.height() << endl;
    m_video_size = video_size;

    m_scale = (double)widget_size.height() / video_size.height();

    int width = (int)(m_scale * video_size.width() + 0.5);
    width = std::min(width, widget_size.width());

    m_display_size = QSize(width, widget_size.height());
    m_offset_x = (widget_size.width() - width + 1) / 2;

    this->resize(widget_size);
}

void VideoOverlay::paintEvent(QPaintEvent*){
    QPainter painter(this);

    double width = m_scale * m_video_size.width();
    double height = m_scale * m_video_size.height();

    SpinLockGuard lg(m_lock, "VideoOverlay::paintEvent()");
//    cout << "paint: " << m_boxes.size() << endl;
    for (const InferenceBox* box : m_boxes){
        painter.setPen(box->color);
//        cout << box->x << " " << box->y << ", " << box->width << " x " << box->height << endl;
//        cout << (int)(width * box->x + m_offset_x + 0.5)
//             << " " << (int)(height * box->y + 0.5)
//             << ", " << (int)(width * box->width + 0.5)
//             << " x " << (int)(height * box->height + 0.5) << endl;
//        cout << painter.pen().width() << endl;

        //  Compute coordinates. Clip so that it stays in-bounds.
        int xmin = std::max((int)(width * box->x + 0.5), 1) + m_offset_x;
        int ymin = std::max((int)(height * box->y + 0.5), 1);
//        int xmax = std::min(xmin + (int)(width * box->width + 0.5), m_display_size.width() - painter.pen().width());
//        int ymax = std::min(ymin + (int)(height * box->height + 0.5), m_display_size.height() - painter.pen().width());
        int xmax = std::min(xmin + (int)(width * box->width + 0.5), m_display_size.width() - 1);
        int ymax = std::min(ymin + (int)(height * box->height + 0.5), m_display_size.height() - 1);

//        cout << "m_video_size.width() = " << m_widget_size.width() << ", xmax = " << xmax << endl;

        painter.drawRect(
            xmin,
            ymin,
            xmax - xmin,
            ymax - ymin
        );
    }
}


}
