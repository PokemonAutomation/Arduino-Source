/*  Video Feed Interface
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoFeedInterface_H
#define PokemonAutomation_VideoFeedInterface_H

#include <QImage>

namespace PokemonAutomation{


struct InferenceBox{
    QColor color;
    double x;
    double y;
    double width;
    double height;

    InferenceBox(
        double p_x, double p_y,
        double p_width, double p_height
    )
        : color(Qt::red)
        , x(p_x), y(p_y)
        , width(p_width), height(p_height)
    {}
    InferenceBox(
        QColor p_color,
        double p_x, double p_y,
        double p_width, double p_height
    )
        : color(p_color)
        , x(p_x), y(p_y)
        , width(p_width), height(p_height)
    {}
};


class VideoFeed{
public:
    //  Do not call this on the main thread or it will deadlock.
    virtual QImage snapshot() = 0;

    //  Add/remove inference boxes.
    virtual void operator+=(const InferenceBox& box) = 0;
    virtual void operator-=(const InferenceBox& box) = 0;

//    virtual void test_draw(){}

};


class InferenceBoxScope : public InferenceBox{
public:
    template <class... Args>
    InferenceBoxScope(
        VideoFeed& feed,
        Args&&... args
    )
        : InferenceBox(std::forward<Args>(args)...)
        , m_feed(feed)
    {
        feed += *this;
    }
    ~InferenceBoxScope(){
        m_feed -= *this;
    }

private:
    VideoFeed& m_feed;
};


}
#endif
