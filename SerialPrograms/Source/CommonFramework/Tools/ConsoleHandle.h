/*  Console Handle
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ConsoleHandle_H
#define PokemonAutomation_ConsoleHandle_H

#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Widgets/VideoOverlay.h"

namespace PokemonAutomation{



class ConsoleHandle{
public:
    ConsoleHandle(
        size_t index,
        BotBase& botbase,
        VideoFeed& video
    )
        : m_index(index)
        , m_context(botbase)
        , m_video(video)
    {}

    size_t index() const{ return m_index; }

    BotBase& botbase(){ return m_context.botbase(); }
    BotBaseContext& context(){ return m_context; }
    VideoFeed& video(){ return m_video; }

    operator BotBase&(){ return m_context.botbase(); }
    operator VideoFeed&(){ return m_video; }
    operator BotBaseContext&(){ return m_context; }

private:
    size_t m_index;
//    BotBase& m_botbase;
    BotBaseContext m_context;
    VideoFeed& m_video;
};





}
#endif


