/*  Camera Base Class
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_CameraBase_H
#define PokemonAutomation_VideoPipeline_CameraBase_H

#include <vector>
#include "Common/Cpp/ImageResolution.h"
#include "VideoFeed.h"

namespace PokemonAutomation{


class Camera{
public:
    virtual ~Camera() = default;

    //  Everything here needs to be thread-safe.

    virtual Resolution current_resolution() const = 0;
    virtual std::vector<Resolution> supported_resolutions() const = 0;
    virtual void set_resolution(const Resolution& resolution) = 0;

    //  This snapshot function will be called asynchronously from many threads
    //  at a very high rate. It is advised to cache snapshots if the video frame
    //  has not changed from the last call.
    //
    //  If "timestamp" is not "WallClock::min()", it will be set to the best
    //  known timestamp of the screenshot that is returned.
    virtual VideoSnapshot snapshot() = 0;
};



}
#endif
