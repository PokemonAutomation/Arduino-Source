/*  Stream History Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
        size_t audio_samples_per_frame,
        size_t audio_frames_per_second,
        std::chrono::seconds window
    ){}
    void set_window(std::chrono::seconds window){}

    bool save(Logger& logger, const std::string& filename) const{
        logger.log("Cannot save stream history: Not implemented.", COLOR_RED);
        return false;
    }

public:
    void on_samples(const float* data, size_t frames){}
    void on_frame(std::shared_ptr<VideoFrame> frame){}

private:
};


}
#endif
