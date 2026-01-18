/*  Snapshot Manager
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_SnapshotManager_H
#define PokemonAutomation_VideoPipeline_SnapshotManager_H

#include <map>
#include <mutex>
#include <condition_variable>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "CommonFramework/Tools/StatAccumulator.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "QVideoFrameCache.h"

namespace PokemonAutomation{

class AsyncTask;

class SnapshotManager{
public:
    ~SnapshotManager();
    SnapshotManager(Logger& logger, QVideoFrameCache& cache);

public:
    VideoSnapshot snapshot_latest_blocking();
    VideoSnapshot snapshot_recent_nonblocking(WallClock min_time);

private:
    static QImage frame_to_image(const QVideoFrame& frame);
    void convert(uint64_t seqnum, QVideoFrame frame, WallClock timestamp) noexcept;
    bool try_dispatch_conversion(uint64_t seqnum, QVideoFrame frame, WallClock timestamp) noexcept;
    void dispatch_conversion(uint64_t seqnum, QVideoFrame frame, WallClock timestamp) noexcept;

    void push_new_screenshot(uint64_t seqnum, VideoSnapshot snapshot);

    struct ObjectsToGC{
        std::vector<std::unique_ptr<AsyncTask>> tasks_to_free;
        std::vector<VideoSnapshot> snapshots_to_free;

        void destroy_now();
    };
    ObjectsToGC cleanup();

private:
    Logger& m_logger;
    QVideoFrameCache& m_cache;

    std::mutex m_lock;
    std::condition_variable m_cv;

    size_t m_active_conversions;
    std::map<uint64_t, std::unique_ptr<AsyncTask>> m_pending_conversions;
    bool m_queued_convert = false;

    uint64_t m_converting_seqnum;

    //  Store the latest converted snapshot + seqnum.
    uint64_t m_converted_seqnum;
    VideoSnapshot m_converted_snapshot;

    //  Keep an archive of older snapshots. The idea here is that we don't want
    //  snapshots to be destroyed in other places (such as the video pivot)
    //  because it's expensive. So instead, we keep a reference here which we
    //  will periodically clear out on the conversion threads.
    std::map<uint64_t, VideoSnapshot> m_converted_snapshot_archive;

    PeriodicStatsReporterI32 m_stats_conversion;
};





}
#endif
