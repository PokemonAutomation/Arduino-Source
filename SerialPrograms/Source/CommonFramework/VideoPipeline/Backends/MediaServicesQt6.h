/*  Media Services (Qt6.5)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_Qt65MediaServices_H
#define PokemonAutomation_VideoPipeline_Qt65MediaServices_H

#include <mutex>
#include <condition_variable>
#include <QCameraDevice>
#include <QMediaDevices>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Concurrency/Thread.h"

namespace PokemonAutomation{


class GlobalMediaServices : public QObject{
public:
    static GlobalMediaServices& instance(){
        static GlobalMediaServices self;
        return self;
    }

    // Get all camera devices' info.
    // Note: to avoid freezing the UI while launching the application, the camera backend when
    // constructed, use a separate thread to query and load camera info. If you call
    // `get_all_cameras()` immidiately after the GlobalMediaServices singleton is constructed,
    // it may not give you all cameras' info.
    QList<QCameraDevice> get_all_cameras(){
        ReadSpinLock lg(m_camera_lock);
        return m_cameras;
    }

    void stop();

private:
    GlobalMediaServices();
    ~GlobalMediaServices();

    void thread_body();

    void refresh_cameras();

private:
    QMediaDevices m_media_devices;

    std::mutex m_sleep_lock;
    std::condition_variable m_cv;

    bool m_stopping = false;
    bool m_refresh_cameras = false;

    SpinLock m_camera_lock;
    QList<QCameraDevice> m_cameras;

    Thread m_thread;
};




}
#endif
