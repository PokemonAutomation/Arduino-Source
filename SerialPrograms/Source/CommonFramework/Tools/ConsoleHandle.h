/*  Console Handle
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ConsoleHandle_H
#define PokemonAutomation_ConsoleHandle_H

#include <memory>
#include "VideoStream.h"

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


class ConsoleHandle : public VideoStream{
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
        BotBase* botbase,
        VideoFeed& video,
        VideoOverlay& overlay,
        AudioFeed& audio,
        const StreamHistorySession& history
    );

    size_t index() const{ return m_index; }

    BotBase& botbase(){ return *m_botbase; }


    operator Logger&(){ return logger(); }
    operator VideoFeed&(){ return video(); }
    operator VideoOverlay&(){ return overlay(); }
    operator AudioFeed&() { return audio(); }
    operator const StreamHistorySession&() const{ return history(); }


private:
    size_t m_index;
    BotBase* m_botbase;

    std::unique_ptr<ThreadUtilizationStat> m_thread_utilization;
};





}
#endif


