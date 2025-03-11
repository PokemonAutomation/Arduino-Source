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
#include "MediaServicesQt6.h"

namespace PokemonAutomation{


GlobalMediaServices::~GlobalMediaServices(){
    {
        std::lock_guard<std::mutex> lg(m_sleep_lock);
        m_stopping = true;
        m_cv.notify_all();
    }
    m_thread.join();
}
GlobalMediaServices::GlobalMediaServices()
    : m_thread(&GlobalMediaServices::thread_body, this)
{
    refresh_cameras();

    m_media_devices.connect(
        &m_media_devices, &QMediaDevices::videoInputsChanged,
        this, [this]{
            std::lock_guard<std::mutex> lg(m_sleep_lock);
            m_refresh_cameras = true;
            m_cv.notify_all();
        }
    );
}


void GlobalMediaServices::thread_body(){
    std::unique_lock<std::mutex> lg(m_sleep_lock);
    while (!m_stopping){
//            global_logger_tagged().log("GlobalMediaServices::thread_body() iteration", COLOR_CYAN);
        if (m_refresh_cameras){
            refresh_cameras();
        }
        m_cv.wait(lg);
    }
}


void GlobalMediaServices::refresh_cameras(){
    try{
        global_logger_tagged().log("Start refreshing camera list...", COLOR_CYAN);
        WallClock start = current_time();

        QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
        {
            WriteSpinLock lg(m_camera_lock);
            m_cameras = std::move(cameras);
        }

        WallClock end = current_time();
        double seconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.;
        global_logger_tagged().log("Done refreshing camera list... " + tostr_fixed(seconds, 3) + " seconds", COLOR_CYAN);
    }catch (std::exception& e){
        global_logger_tagged().log(
            std::string("Refreshing camera list returned exception: ") + e.what(),
            COLOR_RED
        );
    }
    m_refresh_cameras = false;
}




}
#endif
