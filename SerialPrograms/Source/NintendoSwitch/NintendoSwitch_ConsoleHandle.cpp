/*  Console Handle
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/Stats/ThreadUtilizationStats.h"
#include "CommonFramework/InferenceInfra/VisualInferencePivot.h"
#include "CommonFramework/InferenceInfra/AudioInferencePivot.h"
#include "CommonFramework/Recording/StreamHistorySession.h"
#include "NintendoSwitch_ConsoleHandle.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


ConsoleHandle::ConsoleHandle(ConsoleHandle&& x) = default;
ConsoleHandle::~ConsoleHandle(){
    overlay().remove_stat(*m_thread_utilization);
}


ConsoleHandle::ConsoleHandle(
    size_t index,
    Logger& logger,
    BotBase& botbase,
    VideoFeed& video,
    VideoOverlay& overlay,
    AudioFeed& audio,
    const StreamHistorySession& history
)
    : VideoStream(logger, audio, video, history, overlay)
    , m_index(index)
    , m_botbase(botbase)
    , m_thread_utilization(new ThreadUtilizationStat(current_thread_handle(), "Program Thread:"))
{
    overlay.add_stat(*m_thread_utilization);
}






}
