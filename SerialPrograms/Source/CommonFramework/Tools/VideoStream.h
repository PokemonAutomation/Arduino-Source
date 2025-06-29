/*  Video Stream
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      Represents a single video stream. Includes video, audio, as well
 *  as their inference pivots. Also includes a logger and the video overlay.
 *
 *  This is the class that should be passed into inference modules as it
 *  provides all the information needed for inference.
 *
 *  This class has been separated from ConsoleHandle which used to take this
 *  role. But 95% of the uses don't require the controller. And the controller
 *  is messy due to being heterogeneous.
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoStream_H
#define PokemonAutomation_VideoPipeline_VideoStream_H

#include <string>
#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Containers/Pimpl.h"

namespace PokemonAutomation{

class CancellableScope;
class AsyncDispatcher;
class Logger;
class AudioFeed;
class VideoFeed;
class StreamHistorySession;
class VideoOverlay;
class VisualInferencePivot;
class AudioInferencePivot;


class VideoStream{
public:
    VideoStream(VideoStream&& x);
    void operator=(VideoStream&& x) = delete;
    VideoStream(const VideoStream& x) = delete;
    void operator=(const VideoStream& x) = delete;
    ~VideoStream();

public:
    VideoStream(
        Logger& logger,
        AudioFeed& audio,
        VideoFeed& video,
        const StreamHistorySession& history,
        VideoOverlay& overlay
    );

    //  log(string-like msg, Color color = Color())
    //  string-like can be const char* or std::string
    template <class... Args>
    void log(Args&&... args){
        m_logger.log(std::forward<Args>(args)...);
    }

    Logger& logger(){ return m_logger; }

    AudioFeed& audio(){ return m_audio; }
    VideoFeed& video(){ return m_video; }

    const StreamHistorySession& history() const{ return m_history; }
    bool save_stream_history(const std::string& filename);

    VideoOverlay& overlay(){ return m_overlay; }

    VisualInferencePivot& video_inference_pivot(){ return *m_video_pivot; }
    AudioInferencePivot& audio_inference_pivot(){ return *m_audio_pivot; }


public:
    void initialize_inference_threads(CancellableScope& scope, AsyncDispatcher& dispatcher);


private:
    Logger& m_logger;

    AudioFeed& m_audio;
    VideoFeed& m_video;
    const StreamHistorySession& m_history;

    VideoOverlay& m_overlay;

    Pimpl<VisualInferencePivot> m_video_pivot;
    Pimpl<AudioInferencePivot> m_audio_pivot;
};



}
#endif
