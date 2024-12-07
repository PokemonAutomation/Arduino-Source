/*  Console Handle
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ConsoleHandle_H
#define PokemonAutomation_ConsoleHandle_H

#include <memory>
#include "Common/Cpp/AbstractLogger.h"

namespace PokemonAutomation{

class CancellableScope;
class AsyncDispatcher;
class ThreadHandle;
class BotBase;
class VideoFeed;
class VideoOverlay;
class AudioFeed;
class StreamHistorySession;
class ThreadUtilizationStat;
class VisualInferencePivot;
class AudioInferencePivot;


class ConsoleHandle{
public:
    ConsoleHandle(ConsoleHandle&& x);
    void operator=(ConsoleHandle&& x) = delete;
    ConsoleHandle(const ConsoleHandle& x) = delete;
    void operator=(const ConsoleHandle& x) = delete;
    ~ConsoleHandle();

public:
    ConsoleHandle(
        size_t index,
        Logger& logger,
//        ThreadHandle& handle,
        BotBase* botbase,
        VideoFeed& video,
        VideoOverlay& overlay,
        AudioFeed& audio,
        const StreamHistorySession& history
    );

    // log(string-like msg, Color color = Color())
    // string-like can be const char* or std::string
    template <class... Args>
    void log(Args&&... args){
        m_logger.log(std::forward<Args>(args)...);
    }

    size_t index() const{ return m_index; }

    Logger& logger(){ return m_logger; }
    BotBase& botbase(){ return *m_botbase; }
    VideoFeed& video(){ return m_video; }
    VideoOverlay& overlay(){ return m_overlay; }
    AudioFeed& audio(){ return m_audio; }
    bool save_stream_history(const std::string& filename);

    operator Logger&(){ return m_logger; }
    operator VideoFeed&(){ return m_video; }
    operator VideoOverlay&(){ return m_overlay; }
    operator AudioFeed&() { return m_audio; }

    VisualInferencePivot& video_inference_pivot(){ return *m_video_pivot; }
    AudioInferencePivot& audio_inference_pivot(){ return *m_audio_pivot; }


public:
    void initialize_inference_threads(CancellableScope& scope, AsyncDispatcher& dispatcher);

private:
    size_t m_index;
    Logger& m_logger;
    BotBase* m_botbase;
    VideoFeed& m_video;
    VideoOverlay& m_overlay;
    AudioFeed& m_audio;
    const StreamHistorySession& m_history;
    std::unique_ptr<ThreadUtilizationStat> m_thread_utilization;
    std::unique_ptr<VisualInferencePivot> m_video_pivot;
    std::unique_ptr<AudioInferencePivot> m_audio_pivot;
};





}
#endif


