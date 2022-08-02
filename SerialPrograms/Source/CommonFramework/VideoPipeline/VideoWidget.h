/*  Video Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoWidget_H
#define PokemonAutomation_VideoPipeline_VideoWidget_H

#include <vector>
#include <QWidget>
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
    //  If "timestamp" is not null, it will be set to the best known timestamp
    //  of the screenshot that is returned.
    virtual VideoSnapshot snapshot() = 0;
};



//  Base class for the widget that loads the video content.
class VideoWidget : public QWidget{
public:
    using QWidget::QWidget;
    virtual ~VideoWidget() = default;

    //  Get the camera associated with this VideoWidget.
    virtual Camera& camera() = 0;
};



}
#endif
