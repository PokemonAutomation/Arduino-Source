/*  Console Handle
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ConsoleHandle_H
#define PokemonAutomation_ConsoleHandle_H

#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Widgets/VideoOverlayWidget.h"

namespace PokemonAutomation{



class ConsoleHandle{
public:
    ConsoleHandle(
        size_t index,
        Logger& logger,
        BotBase& botbase,
        VideoFeed& video,
        VideoOverlay& overlay
    )
        : m_index(index)
        , m_logger(logger, "Switch " + std::to_string(index))
        , m_context(botbase)
        , m_video(video)
        , m_overlay(overlay)
    {}

    template <class... Args>
    void log(Args&&... args){
        m_logger.log(std::forward<Args>(args)...);
    }

    size_t index() const{ return m_index; }

    Logger& logger(){ return m_logger; }
    BotBase& botbase(){ return m_context.botbase(); }
    BotBaseContext& context(){ return m_context; }
    VideoFeed& video(){ return m_video; }
    VideoOverlay& overlay(){ return m_overlay; }

    operator Logger&(){ return m_logger; }
    operator BotBase&(){ return m_context.botbase(); }
    operator BotBaseContext&(){ return m_context; }
    operator VideoFeed&(){ return m_video; }
    operator VideoOverlay&(){ return m_overlay; }

private:
    size_t m_index;
    TaggedLogger m_logger;
//    BotBase& m_botbase;
    BotBaseContext m_context;
    VideoFeed& m_video;
    VideoOverlay& m_overlay;
};





}
#endif


