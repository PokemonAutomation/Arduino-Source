/*  Console Handle
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ConsoleHandle_H
#define PokemonAutomation_ConsoleHandle_H

#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/Logging/LoggerQt.h"

namespace PokemonAutomation{

class VideoFeed;
class VideoOverlay;
class AudioFeed;


class ConsoleHandle{
public:
    ConsoleHandle(
        size_t index,
        LoggerQt& logger,
        BotBase& botbase,
        VideoFeed& video,
        VideoOverlay& overlay,
        AudioFeed& audio
    )
        : m_index(index)
        , m_logger(logger)
        , m_botbase(botbase)
        , m_video(video)
        , m_overlay(overlay)
        , m_audio(audio)
    {}

    template <class... Args>
    void log(Args&&... args){
        m_logger.log(std::forward<Args>(args)...);
    }

    size_t index() const{ return m_index; }

    LoggerQt& logger(){ return m_logger; }
    BotBase& botbase(){ return m_botbase; }
//    BotBaseContext& context(){ return m_context; }
    VideoFeed& video(){ return m_video; }
    VideoOverlay& overlay(){ return m_overlay; }
    AudioFeed& audio(){ return m_audio; }

    operator LoggerQt&(){ return m_logger; }
//    operator BotBase&(){ return m_context.botbase(); }
//    operator BotBaseContext&(){ return m_context; }
    operator VideoFeed&(){ return m_video; }
    operator VideoOverlay&(){ return m_overlay; }
    operator AudioFeed&() { return m_audio; }

private:
    size_t m_index;
    LoggerQt& m_logger;
    BotBase& m_botbase;
    VideoFeed& m_video;
    VideoOverlay& m_overlay;
    AudioFeed& m_audio;
};





}
#endif


