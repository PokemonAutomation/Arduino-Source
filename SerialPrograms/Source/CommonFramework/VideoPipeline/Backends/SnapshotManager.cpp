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
    std::unique_lock<Mutex> lg(m_lock);
    m_cv.wait(lg, [this]{
        cleanup();
        return m_pending_conversions.empty();
    });
}
SnapshotManager::SnapshotManager(Logger& logger, QVideoFrameCache& cache)
    : m_logger(logger)
    , m_cache(cache)
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
VideoSnapshot SnapshotManager::convert(QVideoFrame frame, WallClock timestamp) noexcept{
    VideoSnapshot snapshot;
    snapshot.timestamp = timestamp;
    try{
        WallClock time0 = current_time();
        snapshot.frame = std::make_shared<const ImageRGB32>(frame_to_image(frame));
        WallClock time1 = current_time();
        WriteSpinLock lg(m_stats_lock);
        m_stats_conversion.report_data(
            m_logger,
            (uint32_t)std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count()
        );
    }catch (...){
        try{
            m_logger.log("Exception thrown while converting QVideoFrame -> QImage.", COLOR_RED);
        }catch (...){}
    }
    return snapshot;
}
void SnapshotManager::convert(uint64_t seqnum, QVideoFrame frame, WallClock timestamp) noexcept{
    VideoSnapshot snapshot = convert(std::move(frame), timestamp);

    ObjectsToGC objects_to_gc;
    {
        std::lock_guard<Mutex> lg(m_lock);
//        cout << "SnapshotManager::convert() - post convert: " << seqnum << endl;

        push_new_screenshot(seqnum, std::move(snapshot));

        if (m_queued_convert){
            m_queued_convert = false;
            seqnum = m_cache.get_latest(frame, timestamp);
            dispatch_conversion(seqnum, std::move(frame), timestamp);
        }

        objects_to_gc = cleanup();
    }
    objects_to_gc.destroy_now();

    m_cv.notify_all();
}
bool SnapshotManager::try_dispatch_conversion(uint64_t seqnum, QVideoFrame frame, WallClock timestamp) noexcept{
    //  Must call under the lock.

    AsyncTask* task;
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

        *task = GlobalThreadPools::computation_realtime().try_dispatch_now(lambda);

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
    try_dispatch_conversion(seqnum, std::move(frame), timestamp);
}

bool SnapshotManager::push_new_screenshot(uint64_t seqnum, VideoSnapshot snapshot){
    if (!m_converted_snapshot_archive.empty() && m_converted_snapshot_archive.rbegin()->first > seqnum){
        return false;
    }
    m_converted_snapshot_archive[seqnum] = snapshot;
    return true;
}
void SnapshotManager::ObjectsToGC::destroy_now(){
    tasks_to_free.clear();
    snapshots_to_free.clear();
}
SnapshotManager::ObjectsToGC SnapshotManager::cleanup(){
    //  Must call under the lock!

    //  Grab and return all objects that are ready to be garbage collected.

    ObjectsToGC ret;

    //  Cleanup finished tasks.
    while (!m_pending_conversions.empty()){
        auto iter = m_pending_conversions.begin();
        if (iter->second.is_finished()){
            ret.tasks_to_free.emplace_back(std::move(iter->second));
            m_pending_conversions.erase(iter);
        }else{
            break;
        }
    }

    //  Walk through the snapshot archive and clear out everything with only
    //  one reference. But leave behind the latest screenshot.

    if (m_converted_snapshot_archive.size() <= 1){
        return ret;
    }

    auto stop = m_converted_snapshot_archive.end();
    --stop;

    for (auto iter = m_converted_snapshot_archive.begin(); iter != stop;){
        if (iter->second.frame.use_count() <= 1){
            ret.snapshots_to_free.emplace_back(std::move(iter->second));
            iter = m_converted_snapshot_archive.erase(iter);
        }else{
            ++iter;
        }
    }

    return ret;
}


VideoSnapshot SnapshotManager::snapshot_latest_blocking(){
    ObjectsToGC objects_to_gc;
    {
        std::unique_lock<Mutex> lg(m_lock);
        objects_to_gc = cleanup();
    }
    objects_to_gc.destroy_now();

    VideoSnapshot snapshot;
    bool notify = false;
    {
        std::unique_lock<Mutex> lg(m_lock);

//        cout << "snapshot_latest_blocking()" << endl;

        //  Already up-to-date. Return it.
        uint64_t seqnum = m_cache.seqnum();
        if (!m_converted_snapshot_archive.empty()){
            auto iter = m_converted_snapshot_archive.rbegin();
            if (seqnum <= iter->first){
//                cout << "snapshot_latest_blocking(): Cached" << endl;
                return iter->second;
            }
        }

        //  Check if we're already converting it.
        if (!m_pending_conversions.empty() && m_pending_conversions.rbegin()->first >= seqnum){
//            cout << "snapshot_latest_blocking(): Already Converting" << endl;
            while (true){
                std::map<uint64_t, VideoSnapshot>::reverse_iterator iter = m_converted_snapshot_archive.rbegin();
                if (iter->first >= seqnum){
                    return iter->second;
                }
                m_cv.wait(lg);
            }
        }


        //  Otherwise, we use this thread to convert the latest frame.
//        cout << "snapshot_latest_blocking(): Convert Now" << endl;

        QVideoFrame frame;
        WallClock timestamp;
        seqnum = m_cache.get_latest(frame, timestamp);

        notify = push_new_screenshot(seqnum, convert(std::move(frame), timestamp));

        snapshot = m_converted_snapshot_archive.rbegin()->second;
    }

    if (notify){
        m_cv.notify_all();
    }

//    cout << "snapshot_latest_blocking(): Convert Now - Done" << endl;
    return snapshot;
}

VideoSnapshot SnapshotManager::snapshot_recent_nonblocking(WallClock min_time){
//    WallClock now = current_time();

    std::lock_guard<Mutex> lg(m_lock);

    //  Already up-to-date. Return it.
    uint64_t seqnum = m_cache.seqnum();
    if (!m_converted_snapshot_archive.empty()){
        auto iter = m_converted_snapshot_archive.rbegin();
        if (seqnum <= iter->first){
//            cout << "snapshot_latest_blocking(): Cached" << endl;
            return iter->second;
        }
    }

    QVideoFrame frame;
    WallClock timestamp;
    seqnum = m_cache.get_latest(frame, timestamp);

    //  Dispatch this frame for conversion.
    dispatch_conversion(seqnum, std::move(frame), timestamp);

    //  No cached snapshot.
    if (m_converted_snapshot_archive.empty()){
        return VideoSnapshot();
    }

    //  Cached snapshot is too old.
    auto iter = m_converted_snapshot_archive.rbegin();
    if (min_time > iter->second.timestamp){
        return VideoSnapshot();
    }

    return iter->second;
}


















}
