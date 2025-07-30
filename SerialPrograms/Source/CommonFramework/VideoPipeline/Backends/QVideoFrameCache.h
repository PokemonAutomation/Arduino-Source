/*  QVideoFrameCache
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      A simple cache that stores the last QVideoFrame from a stream.
 *
 */

#ifndef PokemonAutomation_VideoPipeline_QVideoFrameCache_H
#define PokemonAutomation_VideoPipeline_QVideoFrameCache_H

#include <QVideoFrame>
#include "Common/Cpp/Time.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/Tools/StatAccumulator.h"

//#define PA_PROFILE_QVideoFrameCache

namespace PokemonAutomation{



class QVideoFrameCache{
public:
    QVideoFrameCache(Logger& logger)
#ifdef PA_PROFILE_QVideoFrameCache
        : m_logger(logger)
        , m_last_frame_timestamp(WallClock::min())
#else
        : m_last_frame_timestamp(WallClock::min())
#endif
        , m_last_frame_seqnum(0)
#ifdef PA_PROFILE_QVideoFrameCache
        , m_stats_lock("QVideoFrameCache::push_frame()-Lock", "ms", 1000, std::chrono::seconds(10))
        , m_stats_push_frame("QVideoFrameCache::push_frame()-All", "ms", 1000, std::chrono::seconds(10))
#endif
    {}

    uint64_t seqnum() const{
        return m_last_frame_seqnum.load(std::memory_order_relaxed);
    }
    uint64_t get_latest(QVideoFrame& frame, WallClock& timestamp) const{
        WriteSpinLock lg(m_frame_lock, "QVideoFrameCache::get_latest()");
        frame = m_last_frame;
        timestamp = m_last_frame_timestamp;
        return seqnum();
    }

    bool push_frame(QVideoFrame frame, WallClock timestamp){
#ifdef PA_PROFILE_QVideoFrameCache
        WallClock time0 = current_time();
#endif

        WriteSpinLock lg(m_frame_lock, "QVideoFrameCache::push_frame()");

#ifdef PA_PROFILE_QVideoFrameCache
        {
            WallClock time1 = current_time();
            uint32_t microseconds = (uint32_t)std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count();
            m_stats_lock.report_data(m_logger, microseconds);
        }
#endif

        //  Skip duplicate frames.
        if (frame.startTime() != -1 && frame.startTime() <= m_last_frame.startTime()){
            return false;
        }

        m_last_frame = std::move(frame);
        m_last_frame_timestamp = timestamp;
        uint64_t seqnum = m_last_frame_seqnum.load(std::memory_order_relaxed);
        seqnum++;
        m_last_frame_seqnum.store(seqnum, std::memory_order_relaxed);

#ifdef PA_PROFILE_QVideoFrameCache
        {
            WallClock time1 = current_time();
            uint32_t microseconds = (uint32_t)std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count();
            m_stats_push_frame.report_data(m_logger, microseconds);
        }
#endif

        return true;
    }


private:
#ifdef PA_PROFILE_QVideoFrameCache
    Logger& m_logger;
#endif

    mutable SpinLock m_frame_lock;

    QVideoFrame m_last_frame;
    WallClock m_last_frame_timestamp;
    std::atomic<uint64_t> m_last_frame_seqnum;

#ifdef PA_PROFILE_QVideoFrameCache
    PeriodicStatsReporterI32 m_stats_lock;
    PeriodicStatsReporterI32 m_stats_push_frame;
#endif
};







}
#endif
