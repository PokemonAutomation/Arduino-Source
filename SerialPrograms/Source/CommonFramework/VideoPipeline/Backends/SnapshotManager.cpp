/*  Snapshot Manager
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Concurrency/ReverseLockGuard.h"
#include "SnapshotManager.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{




SnapshotManager::~SnapshotManager(){
    std::unique_lock<std::mutex> lg(m_lock);
    m_cv.wait(lg, [this]{ return m_active_conversions == 0; });
}
SnapshotManager::SnapshotManager(Logger& logger, QVideoFrameCache& cache)
    : m_logger(logger)
    , m_cache(cache)
    , m_active_conversions(0)
    , m_converting_seqnum(0)
    , m_converted_seqnum(0)
    , m_stats_conversion("ConvertFrame", "ms", 1000, std::chrono::seconds(10))
{}


QImage SnapshotManager::frame_to_image(const QVideoFrame& frame){
    QImage image = frame.toImage();
    QImage::Format format = image.format();
    if (format != QImage::Format_ARGB32 && format != QImage::Format_RGB32){
        image = image.convertToFormat(QImage::Format_ARGB32);
    }
    return image;
}




VideoSnapshot SnapshotManager::screenshot_latest_blocking(){
    std::unique_lock<std::mutex> lg(m_lock);

//    cout << "screenshot_latest_blocking()" << endl;

    //  Already up-to-date. Return it.
    uint64_t seqnum = m_cache.seqnum();
    if (seqnum <= m_converted_seqnum){
//        cout << "screenshot_latest_blocking(): Cached" << endl;
        return m_converted_snapshot;
    }

    //  Check if we're already converting it.
    if (m_converting_seqnum >= seqnum){
//        cout << "screenshot_latest_blocking(): Already Converting" << endl;
        m_cv.wait(lg, [=, this]{ return m_converted_seqnum >= seqnum; });
//        cout << "screenshot_latest_blocking(): Already Converting - Done" << endl;
        return m_converted_snapshot;
    }


    //  Otherwise, we use this thread to convert the latest frame.
//    cout << "screenshot_latest_blocking(): Convert Now" << endl;

    QVideoFrame frame;
    WallClock timestamp;
    seqnum = m_cache.get_latest(frame, timestamp);

    m_active_conversions++;
    m_converting_seqnum = seqnum;

    VideoSnapshot snapshot;
    try{
        WallClock time0, time1;
        uint32_t microseconds;
        {
            ReverseLockGuard<std::mutex> lg0(m_lock);
            time0 = current_time();
            snapshot = VideoSnapshot(frame_to_image(frame), timestamp);
            time1 = current_time();
            microseconds = (uint32_t)std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count();
        }
        m_stats_conversion.report_data(m_logger, microseconds);
        m_active_conversions--;
    }catch (...){
        m_active_conversions--;
        m_logger.log("Exception thrown while converting QVideoFrame -> QImage.", COLOR_RED);
        throw;
    }

    if (timestamp > m_converted_snapshot.timestamp){
        m_converted_seqnum = seqnum;
        m_converted_snapshot = std::move(snapshot);
        m_cv.notify_all();
    }

//    cout << "screenshot_latest_blocking(): Convert Now - Done" << endl;
    return m_converted_snapshot;
}




}
