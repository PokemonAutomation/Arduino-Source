/*  Video Feed Interface
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoFeedInterface_H
#define PokemonAutomation_VideoFeedInterface_H

#include <QImage>
#include "Common/Cpp/Time.h"

namespace PokemonAutomation{


struct VideoSnapshot{
    //  The frame itself. Null means no snapshot was available.
    QImage frame;

    //  The timestamp of when the frame was taken.
    //  This will be as close as possible to when the frame was taken.
    WallClock timestamp = WallClock::min();

    operator QImage&(){ return frame; }
};




//  Define basic interface of a video feed to be used
//  by programs.
class VideoFeed{
public:
    //  Can call from anywhere.
    virtual void async_reset_video() = 0;

    //  Do not call this on the main thread or it will deadlock.
    //  Format must be "Format_ARGB32" or "Format_RGB32".
    virtual VideoSnapshot snapshot() = 0;
};







}
#endif
