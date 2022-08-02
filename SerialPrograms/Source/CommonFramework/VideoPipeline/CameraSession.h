/*  Camera Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoPipeline_CameraSession_H
#define PokemonAutomation_VideoPipeline_CameraSession_H

#include <set>
#include <mutex>
#include <condition_variable>
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CameraOption.h"

namespace PokemonAutomation{

class Camera;

class CameraSession{
public:
    struct Listener{
        virtual void camera_startup(Camera& camera) = 0;
        virtual void camera_shutdown() = 0;
    };


public:
    CameraSession(
        CameraOption& option,
        Logger& logger
    );
    ~CameraSession();

    void add_listener(Listener& listener);
    void remove_listener(Listener& listener);


public:
    CameraInfo info() const;
    Resolution resolution() const;

    VideoSnapshot snapshot();

    //  Not thread-safe with camera changing.
    Camera* camera(){ return m_camera.get(); }


public:
    void set_info(const CameraInfo& info);
    void set_resolution(const Resolution& resolution);

    void start_camera();
    void stop_camera();
    void reset();


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

    mutable std::mutex m_lock;
    std::condition_variable m_cv;

    std::unique_ptr<Camera> m_camera;

    std::set<Listener*> m_listeners;
};



}
#endif
