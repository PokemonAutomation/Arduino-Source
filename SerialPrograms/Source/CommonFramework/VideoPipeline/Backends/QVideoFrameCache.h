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

namespace PokemonAutomation{



class QVideoFrameCache{
public:
    QVideoFrameCache()
        : m_last_frame_timestamp(WallClock::min())
        , m_last_frame_seqnum(0)
    {}

    uint64_t seqnum() const{
        return m_last_frame_seqnum.load(std::memory_order_relaxed);
    }
    uint64_t get_latest(QVideoFrame& frame, WallClock& timestamp) const{
        WriteSpinLock lg(m_frame_lock);
        frame = m_last_frame;
        timestamp = m_last_frame_timestamp;
        return seqnum();
    }

    bool push_frame(QVideoFrame frame, WallClock timestamp){
        WriteSpinLock lg(m_frame_lock);

        //  Skip duplicate frames.
        if (frame.startTime() != -1 && frame.startTime() <= m_last_frame.startTime()){
            return false;
        }

        m_last_frame = frame;
        m_last_frame_timestamp = timestamp;
        uint64_t seqnum = m_last_frame_seqnum.load(std::memory_order_relaxed);
        seqnum++;
        m_last_frame_seqnum.store(seqnum, std::memory_order_relaxed);

        return true;
    }


private:
    mutable SpinLock m_frame_lock;

    QVideoFrame m_last_frame;
    WallClock m_last_frame_timestamp;
    std::atomic<uint64_t> m_last_frame_seqnum;
};







}
#endif
