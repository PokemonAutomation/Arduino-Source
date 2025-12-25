/*  Media Services (Qt6)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_MediaServicesQt6_H
#define PokemonAutomation_VideoPipeline_MediaServicesQt6_H

#include <mutex>
#include <condition_variable>
#include <QCameraDevice>
#include <QMediaDevices>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Concurrency/Thread.h"

namespace PokemonAutomation{


// Singleton to provide all available camera info as a list of QCameraDevice
// asynchronously.
// It uses a persistent background thread to load camera info. The thread
// sleeps on a conditional variable until it is notified by a camera update
// event from QMediaDevices.
class GlobalMediaServices : public QObject{
public:
    // Return the singleton GlobalMediaServices.
    // Note: to avoid freezing the UI while launching the GUI application, when instance() is
    // first called, it constructed a static GlobalMediaServices object and uses a separate
    // thread to query and load camera info. If you call `instance()::get_all_cameras()`
    // the first time the thread does not finish immidiately and it may not give you all
    // the camera info.
    // The proper usage is to call `instance()::get_all_cameras()` immidiately after
    // starting the program to give time for the thread to finish before the UI is constructed
    // and queries the cameras again.
    static GlobalMediaServices& instance(){
        static GlobalMediaServices self;
        return self;
    }

    // Get all camera devices' info (thread safe: you can call this from any thread).
    // Note: to avoid freezing the UI while launching the GUI application, when instance() is
    // first called, it constructed a static GlobalMediaServices object and uses a separate
    // thread to query and load camera info. If you call `instance()::get_all_cameras()`
    // the first time the thread does not finish immidiately and it may not give you all
    // the camera info.
    // The proper usage is to call `instance()::get_all_cameras()` immidiately after
    // starting the program to give time for the thread to finish before the UI is constructed
    // and queries the cameras again.
    QList<QCameraDevice> get_all_cameras(){
        ReadSpinLock lg(m_camera_lock);
        return m_cameras;
    }

    // Get all camera devices' info (thread safe: you can call this from any thread).
    // Blocks the current thread until the camera info is queried, unlike `get_all_cameras()`
    // which is non-blocking.
    // Its usage is to be called at a command-line application/library to ensure you get
    // the current up-to-date camera info.
    QList<QCameraDevice> get_all_cameras_blocking();

    // Shut down the service. Wake up and join the persistent worker thread. After this
    // function is called, you are not supposed to query camera info any more from this
    // class.
    void stop();

private:
    GlobalMediaServices();
    ~GlobalMediaServices();

    // Worker thread loop.
    void thread_body();

    // Called in worker thread main loop when `m_refresh_cameras` is true
    // to load latest camera info and store it into `m_media_devices` (thread-safe:
    // it uses `m_camera_lock`).
    void refresh_cameras();

private:
    QMediaDevices m_media_devices;

    // ========================================================================
    // LOCK #1: Worker Thread Sleep/Wake Synchronization
    // ========================================================================

    // m_sleep_lock and m_cv work together as a classic condition variable pattern:
    // - m_sleep_lock protects m_stopping and m_refresh_cameras flags
    // - m_cv allows worker thread to sleep until signaled by camera update events
    //   from QMediaDevices or from shut down request.
    std::mutex m_sleep_lock;
    // m_sleep_lock and m_cv work together as a classic condition variable pattern:
    // - m_sleep_lock protects m_stopping and m_refresh_cameras flags
    // - m_cv allows worker thread to sleep until signaled by camera update events
    //   from QMediaDevices or from shut down request.
    std::condition_variable m_cv;

    // Worker thread flag (protected by m_sleep_lock):
    // Set by main thread to tell worker to exit.
    bool m_stopping = false;
    // Worker thread flag (protected by m_sleep_lock):
    // Set by Qt signal or constructor to request querying newest camera info.
    bool m_refresh_cameras = false;

    // ========================================================================
    // LOCK #2: Camera Data Access Protection (separate from worker lock!)
    // ========================================================================

    // Reader-writer SpinLock for camera list access (protect `m_cameras`):
    // - Allows multiple concurrent readers (get_all_cameras() calls)
    // - Allows a single writer (refresh_cameras()) to have exclusive access
    // - Kept separate from `m_sleep_lock` so reads don't block on worker
    //   thread's CV wait
    SpinLock m_camera_lock;
    // Internal cache of all the camera info protected by SpinLock `m_camera_lock`.
    // - A single writer (refresh_cameras()) writes newest data to it.
    // - Multiple readers (get_all_cameras() calls from any thread) gets the data
    //   efficiently.
    QList<QCameraDevice> m_cameras;

    // Background persistent worker thread that waits for camera refresh requests
    Thread m_thread;
};




}
#endif
