/*  Stream History Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Implement by keeping an in-memory ring of *encoded* video packets
 *  (hardware H.264, with ffmpeg's built-in LGPL MPEG-4 as a fallback)
 *  and AAC audio packets. Nothing is written to storage until save() is
 *  called, at which point the ring is muxed into a single MP4/MKV file.
 *
 *  This tracker is only compiled when PA_STREAM_HISTORY_FFMPEG is defined
 *  (CMake option -DPA_STREAM_HISTORY_FFMPEG=ON), which requires the ffmpeg
 *  shared libraries.
 *
 */

#ifndef PokemonAutomation_StreamHistoryTracker_Ring_H
#define PokemonAutomation_StreamHistoryTracker_Ring_H

#include <memory>
#include <string>
#include "Common/Cpp/Time.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"

namespace PokemonAutomation{


class StreamHistoryTracker_Ring{
public:
    ~StreamHistoryTracker_Ring();
    StreamHistoryTracker_Ring(
        Logger& logger,
        std::chrono::seconds window,
        size_t audio_samples_per_frame,
        size_t audio_frames_per_second,
        bool has_video
    );
    void set_window(std::chrono::seconds window);

    bool save(const std::string& filename) const;

public:
    void on_samples(const float* data, size_t frames);
    void on_frame(std::shared_ptr<const VideoFrame> frame);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};



}
#endif
