/*  Snapshot Manager
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Concurrency/ReverseLockGuard.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
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
void SnapshotManager::convert(uint64_t seqnum, QVideoFrame frame, WallClock timestamp) noexcept{
    VideoSnapshot snapshot;
    snapshot.timestamp = timestamp;
    try{
        WallClock time0 = current_time();
        snapshot.frame = std::make_shared<const ImageRGB32>(frame_to_image(frame));
        WallClock time1 = current_time();
        uint32_t microseconds = (uint32_t)std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count();
        m_stats_conversion.report_data(m_logger, microseconds);
    }catch (...){
        try{
            m_logger.log("Exception thrown while converting QVideoFrame -> QImage.", COLOR_RED);
        }catch (...){}
    }

    std::lock_guard<std::mutex> lg(m_lock);
//    cout << "SnapshotManager::convert() - post convert: " << seqnum << endl;

    if (m_converted_seqnum < seqnum){
        m_converted_seqnum = seqnum;
        m_converted_snapshot = std::move(snapshot);
    }

    m_active_conversions--;

    if (m_queued_convert){
        m_queued_convert = false;
        seqnum = m_cache.get_latest(frame, timestamp);
        if (m_converting_seqnum < seqnum){
            dispatch_conversion(seqnum, std::move(frame), timestamp);
        }
    }

    //  Warning: The moment we release the lock with (m_active_conversions == 0),
    //  this class can be immediately destructed.

    //  Therefore it is not safe to notify after releasing the lock.
    m_cv.notify_all();

    //  Lock is implicitly released here.
}
bool SnapshotManager::try_dispatch_conversion(uint64_t seqnum, QVideoFrame frame, WallClock timestamp) noexcept{
    //  Must call under the lock.

    std::unique_ptr<AsyncTask>* task;
    try{
        task = &m_pending_conversions[seqnum];

        //  This frame is already being converted.
        if (*task){
            return false;
        }
    }catch (...){
        return false;
    }

    try{
        std::function<void()> lambda = [=, this, frame = std::move(frame)](){
            convert(seqnum, std::move(frame), timestamp);
        };

        *task = GlobalThreadPools::realtime_inference().try_dispatch(lambda);

        //  Dispatch was successful. We're done.
        if (*task){
            return true;
        }

        //  Dispatch failed. Queue it for later.
        m_queued_convert = true;
    }catch (...){}

    m_pending_conversions.erase(seqnum);

    return false;
}
void SnapshotManager::dispatch_conversion(uint64_t seqnum, QVideoFrame frame, WallClock timestamp) noexcept{
    //  Must call under the lock.
    m_active_conversions++;

    if (!try_dispatch_conversion(seqnum, std::move(frame), timestamp)){
        m_active_conversions--;
    }

    //  Cleanup finished tasks.
    while (!m_pending_conversions.empty()){
        auto iter = m_pending_conversions.begin();
        if (iter->second->is_finished()){
            m_pending_conversions.erase(iter);
        }else{
            break;
        }
    }
}



VideoSnapshot SnapshotManager::snapshot_latest_blocking(){
    std::unique_lock<std::mutex> lg(m_lock);

//    cout << "snapshot_latest_blocking()" << endl;

    //  Already up-to-date. Return it.
    uint64_t seqnum = m_cache.seqnum();
    if (seqnum <= m_converted_seqnum){
//        cout << "snapshot_latest_blocking(): Cached" << endl;
        return m_converted_snapshot;
    }

    //  Check if we're already converting it.
    if (m_converting_seqnum >= seqnum){
//        cout << "snapshot_latest_blocking(): Already Converting" << endl;
        m_cv.wait(lg, [=, this]{ return m_converted_seqnum >= seqnum; });
//        cout << "snapshot_latest_blocking(): Already Converting - Done" << endl;
        return m_converted_snapshot;
    }


    //  Otherwise, we use this thread to convert the latest frame.
//    cout << "snapshot_latest_blocking(): Convert Now" << endl;

    QVideoFrame frame;
    WallClock timestamp;
    seqnum = m_cache.get_latest(frame, timestamp);

//    m_active_conversions++;
    m_converting_seqnum = seqnum;

    VideoSnapshot snapshot;
    try{
        uint32_t microseconds;
        {
            ReverseLockGuard<std::mutex> lg0(m_lock);
            WallClock time0 = current_time();
            snapshot = VideoSnapshot(frame_to_image(frame), timestamp);
            WallClock time1 = current_time();
            microseconds = (uint32_t)std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count();
        }
        m_stats_conversion.report_data(m_logger, microseconds);
//        m_active_conversions--;
    }catch (...){
//        m_active_conversions--;
        m_logger.log("Exception thrown while converting QVideoFrame -> QImage.", COLOR_RED);
        throw;
    }

    m_converted_seqnum = seqnum;

    if (timestamp > m_converted_snapshot.timestamp){
        m_converted_snapshot = std::move(snapshot);
        m_cv.notify_all();
    }

//    cout << "snapshot_latest_blocking(): Convert Now - Done" << endl;
    return m_converted_snapshot;
}

VideoSnapshot SnapshotManager::snapshot_recent_nonblocking(WallClock min_time){
//    WallClock now = current_time();

    std::lock_guard<std::mutex> lg(m_lock);

//    cout << "snapshot_recent_nonblocking()" << endl;

    //  Already up-to-date. Return it.
    uint64_t seqnum = m_cache.seqnum();
    if (seqnum <= m_converted_seqnum){
//        cout << "snapshot_recent_nonblocking(): Up-to-date" << endl;
        return m_converted_snapshot;
    }

    QVideoFrame frame;
    WallClock timestamp;
    seqnum = m_cache.get_latest(frame, timestamp);

    //  Dispatch this frame for conversion.
    if (m_converting_seqnum < seqnum){
//        cout << "snapshot_recent_nonblocking(): Dispatching..." << endl;
        dispatch_conversion(seqnum, std::move(frame), timestamp);
    }

    //  Cached snapshot isn't too old. Return it.

    if (min_time <= m_converted_snapshot.timestamp){
//        cout << "snapshot_recent_nonblocking(): Good..." << endl;
        return m_converted_snapshot;
    }else{
//        cout << "snapshot_recent_nonblocking(): Too old..." << endl;
        return VideoSnapshot();
    }

#if 0
    std::chrono::microseconds window(m_stats_conversion.max());
//    cout << "window = " << window.count() << endl;

    WallClock oldest_allowed = now - 2 * window;
//    cout << "snapshot_recent_nonblocking(): " << m_converted_seqnum << endl;

    if (m_converted_snapshot.timestamp >= oldest_allowed){
//        cout << "snapshot_recent_nonblocking(): Good..." << endl;
        return m_converted_snapshot;
    }

//    cout << "snapshot_recent_nonblocking(): Too old..." << endl;
    return VideoSnapshot();
#endif
}


















}
