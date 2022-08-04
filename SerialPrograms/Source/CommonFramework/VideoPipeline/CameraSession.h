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

#include <set>
#include "Common/Cpp/SpinLock.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CameraOption.h"

namespace PokemonAutomation{

class Logger;
class Camera;

class CameraSession : public VideoFeed{
public:
    struct Listener{
        virtual void camera_startup(Camera& camera) = 0;
        virtual void camera_shutdown() = 0;
    };

    void add_listener(Listener& listener);
    void remove_listener(Listener& listener);


public:
    CameraSession(
        CameraOption& option,
        Logger& logger
    );
    ~CameraSession();


public:
    CameraInfo info() const;
    Resolution resolution() const;

    virtual void reset() override;
    virtual VideoSnapshot snapshot() override;

    //  Not thread-safe with camera changing.
    Camera* camera(){ return m_camera.get(); }


public:
    void set_info(const CameraInfo& info);
    void set_resolution(const Resolution& resolution);

    void start_camera();
    void stop_camera();


public:
    bool allow_snapshots() const;
    void set_allow_snapshots(bool allow);


private:
    //  Nothing here is thread-safe.
    void push_camera_startup();
    void push_camera_shutdown();


private:
    CameraOption& m_option;
    Logger& m_logger;

    std::atomic<bool> m_allow_snapshots;

    mutable SpinLock m_lock;

    std::unique_ptr<Camera> m_camera;

    std::set<Listener*> m_listeners;
};



}
#endif
