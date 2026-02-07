/*  Media Services (Qt6.5)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QtGlobal>
#if QT_VERSION_MAJOR == 6

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Time.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "MediaServicesQt6.h"

namespace PokemonAutomation{


GlobalMediaServices::GlobalMediaServices()
    : m_refresh_cameras(true)  // Request initial camera refresh on startup
{
    // Comment out: Don't refresh on main thread (would freeze UI on startup)
    // refresh_cameras();

    // Connect Qt signal to lambda that wakes worker thread when cameras change
    m_media_devices.connect(
        &m_media_devices, &QMediaDevices::videoInputsChanged,
        this, [this]{
            // LOCK ACQUISITION: protect m_refresh_cameras and notify worker thread
            // waiting on the conditional variable m_cv.
            std::lock_guard<Mutex> lg(m_sleep_lock);
            m_refresh_cameras = true;  // Set work flag
            m_cv.notify_all();         // Wake worker thread from cv.wait()
        }
    );

    // Start the persistent background worker thread
    m_thread = GlobalThreadPools::unlimited_normal().blocking_dispatch([this]{
        thread_body();
    });
}
GlobalMediaServices::~GlobalMediaServices(){
    stop();
}

void GlobalMediaServices::stop(){
    if (!m_thread){
        return;
    }

    {
        // LOCK ACQUISITION: acquire m_sleep_lock to set m_stopping and notify
        // worker thread waiting on the conditional variable m_cv.
        std::lock_guard<Mutex> lg(m_sleep_lock);
        m_stopping = true;      // Tell worker to exit loop
        m_cv.notify_all();      // Wake worker from cv.wait() if sleeping
    }

    // Wait for the worker thread to finish its current work and join
    m_thread.reset();
}


void GlobalMediaServices::thread_body(){
    // LOCK ACQUISITION: Worker thread holds m_sleep_lock for entire loop
    // to protect reads of m_stopping and m_refresh_cameras.
    // unique_lock is used (not lock_guard) because cv.wait() needs to unlock/relock
    std::unique_lock<Mutex> lg(m_sleep_lock);

    while (!m_stopping){
//            global_logger_tagged().log("GlobalMediaServices::thread_body() iteration", COLOR_CYAN);

        // Check if camera refresh was requested
        if (m_refresh_cameras){
            // IMPORTANT: refresh_cameras() will acquire m_camera_lock (LOCK #2)
            // We currently hold m_sleep_lock (LOCK #1), so we have both locks briefly
            // This is safe because:
            // - Lock order is always: m_sleep_lock -> m_camera_lock
            // - No other code path takes them in reverse order
            refresh_cameras();
        }

        // CONDITION VARIABLE WAIT - cv.wait(lg)
        // 1. Releases m_sleep_lock (allows other threads to set flags and notify)
        // 2. Puts thread to sleep until m_cv.notify_all() is called
        // 3. When notified, re-acquires m_sleep_lock before returning
        m_cv.wait(lg);
    }
    // Loop exits when m_stopping is true (set by stop() function)
    // Lock is released when lg goes out of scope
}


// PRECONDITION: Called with m_sleep_lock already held by worker thread
void GlobalMediaServices::refresh_cameras(){
    try{
        global_logger_tagged().log("Start refreshing camera list...", COLOR_CYAN);
        WallClock start = current_time();

        // Query Qt for camera devices - this is SLOW and can take seconds!
        // NOTE: This happens OUTSIDE the camera lock, so readers can still
        // access the old camera list while we query for the new one.
        QList<QCameraDevice> cameras = QMediaDevices::videoInputs();

        // LOCK ACQUISITION: Briefly acquire camera lock to update the cache
        // `m_cameras`. This is the ONLY place we write to `m_cameras`.
        {
            WriteSpinLock lg(m_camera_lock, PA_CURRENT_FUNCTION);
            m_cameras = std::move(cameras);
        }
        // Camera lock released - readers can now see updated list

        WallClock end = current_time();
        double seconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.;
        global_logger_tagged().log("Done refreshing camera list... " + tostr_fixed(seconds, 3) + " seconds", COLOR_CYAN);
    }catch (std::exception& e){
        global_logger_tagged().log(
            std::string("Refreshing camera list returned exception: ") + e.what(),
            COLOR_RED
        );
    }

    // Clear the refresh flag (protected by m_sleep_lock, which we still hold)
    m_refresh_cameras = false;
}


QList<QCameraDevice> GlobalMediaServices::get_all_cameras_blocking(){
    {
        // LOCK ACQUISITION: protect m_stopping and m_refresh_cameras.
        // - If the persistent worker thread has finished its update and is now
        //   sleeping, m_refresh_cameras should be false and this function just
        //   returns the camera info in the cache m_cameras by calling
        //   get_all_cameras().
        // - If the persistent thread has not yet finished updating, this function
        //   will wait on m_sleep_lock and end up with m_refresh_cameras to be true
        //   when getting the lock: same situation as above.
        // - If the function gets the lock before the persistent thread can update
        //   the camera, m_refresh_cameras is false, so this function will do the
        //   thread's work and load the camera info in a blocking way. After this
        //   function returns, the persistent thread sees m_refresh_cameras is now
        //   true and will avoid doing the same camera update.
        std::unique_lock<Mutex> lg(m_sleep_lock);
        if (!m_stopping && m_refresh_cameras){
            refresh_cameras();
        }
    }
    return get_all_cameras();
}


}
#endif
