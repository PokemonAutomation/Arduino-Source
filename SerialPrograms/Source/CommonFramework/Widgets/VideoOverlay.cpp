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
        painter.drawRect(
            (int)(width * box->x + m_offset_x + 0.5),
            (int)(height * box->y + 0.5),
            (int)(width * box->width + 0.5),
            (int)(height * box->height + 0.5)
        );
    }
}


}
