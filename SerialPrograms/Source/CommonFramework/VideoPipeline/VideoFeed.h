/*  Video Feed Interface
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoFeedInterface_H
#define PokemonAutomation_VideoFeedInterface_H

#include "Common/Cpp/Time.h"

class QImage;

namespace PokemonAutomation{


//  Define basic interface of a video feed to be used
//  by programs.
class VideoFeed{
public:
    //  Can call from anywhere.
    virtual void async_reset_video() = 0;

    //  Do not call this on the main thread or it will deadlock.
    //  Format must be "Format_ARGB32" or "Format_RGB32".
    virtual QImage snapshot(WallClock* timestamp = nullptr) = 0;
};







}
#endif
