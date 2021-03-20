/*  Video Feed Interface
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoFeedInterface_H
#define PokemonAutomation_VideoFeedInterface_H

#include <QImage>
#include "CommonFramework/Inference/InferenceTypes.h"

namespace PokemonAutomation{


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
