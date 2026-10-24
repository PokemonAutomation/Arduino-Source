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
        //  Whether audio is captured at all; this tracker records it at a
        //  fixed 48 kHz / stereo shape, independent of the input's format
        //  (see push_samples()).
        bool has_audio,
        bool has_video
    );
    void set_window(std::chrono::seconds window);

    bool save(const std::string& filename) const;

public:
    //  Audio input: split (planar) float at the tracker's fixed 48 kHz /
    //  stereo shape, delivered by the audio pipeline's working stream
    //  (StreamHistorySession attaches the listener at that shape). Channel
    //  c starts at data[c * stride] and runs for samples (stride is the
    //  per-channel buffer length, 0 for the single-channel pass-through).
    //  Called on the audio pipeline's producing thread.
    void push_samples(size_t samples, size_t stride, const float* data);
    void on_frame(std::shared_ptr<const VideoFrame> frame);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};



}
#endif
