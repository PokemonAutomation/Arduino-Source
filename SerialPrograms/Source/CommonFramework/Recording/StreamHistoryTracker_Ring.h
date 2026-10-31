/*  Stream History Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Implemented by keeping an in-memory ring of *encoded* video packets
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
#include "Common/Cpp/Containers/Pimpl.h"
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
    //  Changes the history length and evicts what falls outside it. Any
    //  thread. (StreamHistorySession does not call it: it reads the history
    //  length once and applies a new one only with a new tracker.)
    void set_window(std::chrono::seconds window);

    //  Muxes the current history into an MP4 (or MKV, by the filename's
    //  extension) file. Blocks until the file is written; any thread,
    //  concurrently with the recording. Returns false (and logs) on failure
    //  or if nothing was recorded.
    bool save(const std::string& filename) const;

public:
    //  Audio input: split (planar) float at the tracker's fixed 48 kHz /
    //  stereo shape, delivered by the audio pipeline's working stream
    //  (StreamHistorySession attaches the listener at that shape). Channel
    //  c starts at data[c * stride] and runs for samples (stride is the
    //  per-channel buffer length). Encodes AAC inline. Called on the audio
    //  pipeline's producing thread.
    void push_samples(size_t samples, size_t stride, const float* data);

    //  Video input. Throttles the frames to the configured FPS and queues
    //  them for the encoder thread (dropping the oldest when it falls
    //  behind); returns without encoding. Called on the video thread.
    void on_frame(std::shared_ptr<const VideoFrame> frame);

public:
    //  For tests.

    //  push_samples() with the capture time of the last sample given
    //  explicitly instead of read from the clock, so that tests can drive
    //  audio and video on one simulated timeline.
    void push_samples(size_t samples, size_t stride, const float* data, WallClock now);

    //  Blocks until every queued video frame has been encoded into the ring.
    //  on_frame() only queues frames for the encoder thread (and drops the
    //  oldest beyond a small backlog), so tests wait on this between frames.
    void wait_until_idle();

private:
    struct Data;
    Pimpl<Data> m_data;
};



}
#endif
