/*  Stream History Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Implement by running two recordings in parallel.
 *  When each reaches 2X seconds, delete it start over.
 *  Both recordings are offset by X seconds - thus guaranteeing
 *  that the last X seconds are available.
 *
 */

#ifndef PokemonAutomation_StreamHistoryTracker_ParallelStreams_H
#define PokemonAutomation_StreamHistoryTracker_ParallelStreams_H

#include <map>
#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Qt/Redispatch.h"
#include "CommonFramework/VideoPipeline/Backends/VideoFrameQt.h"
#include "StreamRecorder.h"

//  REMOVE
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{









class StreamHistoryTracker{
    static constexpr size_t PARALLEL_RECORDINGS = 2;

public:
    StreamHistoryTracker(
        Logger& logger,
        std::chrono::seconds window,
        size_t audio_samples_per_frame,
        size_t audio_frames_per_second,
        bool has_video
    )
        : m_logger(logger)
        , m_window(window)    //  REMOVE
//        , m_window(100)
        , m_audio_samples_per_frame(audio_samples_per_frame)
        , m_audio_frames_per_second(audio_frames_per_second)
        , m_has_video(has_video)
    {
        update_streams(current_time());
    }
    void set_window(std::chrono::seconds window){
        SpinLockGuard lg(m_lock);
        m_window = window;
        update_streams(current_time());
    }

    bool save(const std::string& filename){
        std::unique_ptr<StreamRecording> recording;
        {
            SpinLockGuard lg(m_lock);
            if (m_recordings.empty()){
                m_logger.log("Cannot save stream history. Recording is not enabled.", COLOR_RED);
                return false;
            }

            m_logger.log("Saving stream history...", COLOR_BLUE);

            auto iter = m_recordings.begin();
            recording = std::move(iter->second);
            m_recordings.erase(iter);
            update_streams(current_time());
        }
        return recording->stop_and_save(filename);
    }


    void on_samples(const float* samples, size_t frames){
        WallClock now = current_time();
        SpinLockGuard lg(m_lock);
        for (auto& item : m_recordings){
            item.second->push_samples(now, samples, frames);
        }
        update_streams(now);
    }
    void on_frame(std::shared_ptr<const VideoFrame> frame){
        WallClock now = current_time();
        SpinLockGuard lg(m_lock);
        for (auto& item : m_recordings){
            item.second->push_frame(frame);
        }
        update_streams(now);
    }


private:
    void update_streams(WallClock current_time){
//        dispatch_to_main_thread([this, current_time]{
            internal_update_streams(current_time);
//        });
    }
    void internal_update_streams(WallClock current_time){
//        cout << "streams = " << m_recordings.size() << endl;

        //  Must call under the lock.
        WallClock threshold = current_time - m_window;

        //  Clear any streams that are not needed any more.
        //  A stream is not needed anymore when there is a newer stream that
        //  is at least "m_window" long.
        if (PARALLEL_RECORDINGS >= 2){
            while (m_recordings.size() >= PARALLEL_RECORDINGS){
                auto first = m_recordings.begin();
                auto second = first;
                ++second;
                if (second->first < threshold){
//                    cout << "removing recording..." << endl;
                    m_recordings.erase(first);
                }else{
                    break;
                }
            }
        }

        //  If all recordings start in the future, reset everything.
        if (!m_recordings.empty() && m_recordings.begin()->first > current_time){
            m_recordings.clear();
        }

        //  Add recordings until we've reached the desired amount.
        while (m_recordings.size() < PARALLEL_RECORDINGS){
//            cout << "adding recording = " << m_recordings.size() << endl;
            //  If no recordings are live, start it now.
            //  Otherwise, schedule to start "m_interval" after the start
            //  of the newest recording.
            WallClock start_time = m_recordings.empty()
                ? current_time
                : m_recordings.rbegin()->first + m_window;

//            cout << std::chrono::duration_cast<std::chrono::milliseconds>(start_time - current_time).count() / 1000. << endl;

            m_recordings.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(start_time),
                std::forward_as_tuple(new StreamRecording(
                    m_logger, std::chrono::milliseconds(500),
                    start_time,
                    m_audio_samples_per_frame,
                    m_audio_frames_per_second,
                    m_has_video
                ))
            );
        }
    }


private:
    Logger& m_logger;
    mutable SpinLock m_lock;
    std::chrono::milliseconds m_window;
    const size_t m_audio_samples_per_frame;
    const size_t m_audio_frames_per_second;
    const bool m_has_video;
    std::map<WallClock, std::unique_ptr<StreamRecording>> m_recordings;
};









}
#endif
