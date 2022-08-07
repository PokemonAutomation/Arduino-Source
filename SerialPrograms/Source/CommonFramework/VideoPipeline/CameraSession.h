/*  Camera Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      CameraSession represents a live camera. It holds onto a Camera object
 *  and can be started/stopped/queried, etc...
 *
 *  This class is not responsible for any UI. However, any changes made to this
 *  class will be forwarded to any UI components that are attached to it.
 *
 *  The UI that allows a user to control this class is in CameraSelectorWidget.
 *  The actual display of the video from the camera is in VideoDisplayWidget.
 *
 */

#ifndef PokemonAutomation_VideoPipeline_CameraSession_H
#define PokemonAutomation_VideoPipeline_CameraSession_H

#include <vector>
#include "Common/Cpp/ImageResolution.h"
#include "CommonFramework/VideoPipeline/CameraInfo.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"

class QWidget;

namespace PokemonAutomation{

class VideoWidget;


class CameraSession : public VideoFeed{
public:
    struct Listener{
        //  Sent before the camera shuts down. Listeners should drop their
        //  references to the internal camera implementation before returning.
        virtual void shutdown() = 0;

        virtual void new_source(const CameraInfo& device, Resolution resolution) = 0;  //  Send after a new camera goes up.
        virtual void resolution_change(Resolution resolution) = 0;
    };
    virtual void add_listener(Listener& listener) = 0;
    virtual void remove_listener(Listener& listener) = 0;


public:
    virtual ~CameraSession() = default;

    virtual void reset() = 0;
    virtual void set_source(CameraInfo device) = 0;
    virtual void set_resolution(Resolution resolution) = 0;

    virtual CameraInfo current_device() const = 0;
    virtual Resolution current_resolution() const = 0;
    virtual std::vector<Resolution> supported_resolutions() const = 0;

    virtual void set_allow_snapshots(bool allow) = 0;

    //  This snapshot function will be called asynchronously from many threads
    //  at a very high rate. It is advised to cache snapshots if the video frame
    //  has not changed from the last call.
    //
    //  If "timestamp" is not "WallClock::min()", it will be set to the best
    //  known timestamp of the screenshot that is returned.
    virtual VideoSnapshot snapshot() = 0;


public:
    virtual VideoWidget* make_QWidget(QWidget* parent){ return nullptr; };
};




}
#endif
