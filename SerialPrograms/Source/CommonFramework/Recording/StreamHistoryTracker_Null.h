/*  Stream History Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  A null implementation that will compile before Qt 6.8.
 *
 */

#ifndef PokemonAutomation_StreamHistoryTracker_Null_H
#define PokemonAutomation_StreamHistoryTracker_Null_H

#include "Common/Compiler.h"
#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/VideoPipeline/Backends/VideoFrameQt.h"

namespace PokemonAutomation{


class StreamHistoryTracker{
public:
    StreamHistoryTracker(
        Logger& logger,
        std::chrono::seconds window,
        size_t audio_samples_per_frame,
        size_t audio_frames_per_second,
        bool has_video
    )
        : m_logger(logger)
    {}
    void set_window(std::chrono::seconds window){}

    bool save(const std::string& filename) const{
        m_logger.log("Cannot save stream history: Not implemented.", COLOR_RED);
        return false;
    }

public:
    void on_samples(const float* data, size_t frames){}
    void on_frame(std::shared_ptr<const VideoFrame> frame){}

private:
    Logger& m_logger;
};


}
#endif
