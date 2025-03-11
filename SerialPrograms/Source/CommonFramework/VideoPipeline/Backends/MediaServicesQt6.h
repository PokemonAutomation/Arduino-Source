/*  Media Services (Qt6.5)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_Qt65MediaServices_H
#define PokemonAutomation_VideoPipeline_Qt65MediaServices_H

#include <mutex>
#include <condition_variable>
#include <thread>
#include <QCameraDevice>
#include <QMediaDevices>
#include "Common/Cpp/Concurrency/SpinLock.h"

namespace PokemonAutomation{


class GlobalMediaServices : public QObject{
public:
    static GlobalMediaServices& instance(){
        static GlobalMediaServices self;
        return self;
    }

    QList<QCameraDevice> get_all_cameras(){
        ReadSpinLock lg(m_camera_lock);
        return m_cameras;
    }

private:
    ~GlobalMediaServices();
    GlobalMediaServices();

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

    std::thread m_thread;
};




}
#endif
