/*  Snapshot Manager
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_SnapshotManager_H
#define PokemonAutomation_VideoPipeline_SnapshotManager_H

#include <mutex>
#include <condition_variable>
#include "Common/Cpp/AbstractLogger.h"
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

private:
    Logger& m_logger;
    QVideoFrameCache& m_cache;

    std::mutex m_lock;
    std::condition_variable m_cv;

    size_t m_active_conversions;
    std::map<uint64_t, std::unique_ptr<AsyncTask>> m_pending_conversions;
    bool m_queued_convert = false;

    uint64_t m_converting_seqnum;

    uint64_t m_converted_seqnum;
    VideoSnapshot m_converted_snapshot;

    PeriodicStatsReporterI32 m_stats_conversion;
};





}
#endif
