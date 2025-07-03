/*  Console Handle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/Stats/ThreadUtilizationStats.h"
#include "CommonTools/InferencePivots/VisualInferencePivot.h"
#include "CommonTools/InferencePivots/AudioInferencePivot.h"
#include "CommonFramework/Recording/StreamHistorySession.h"
#include "NintendoSwitch_ConsoleHandle.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


//ConsoleHandle::ConsoleHandle(ConsoleHandle&& x) = default;
ConsoleHandle::~ConsoleHandle(){
    overlay().remove_stat(*m_thread_utilization);
    overlay().remove_stat(*m_normal_inference_utilization);
    overlay().remove_stat(*m_realtime_inference_utilization);
}


ConsoleHandle::ConsoleHandle(
    size_t index,
    Logger& logger,
    AbstractController& controller,
    VideoFeed& video,
    VideoOverlay& overlay,
    AudioFeed& audio,
    const StreamHistorySession& history
)
    : VideoStream(logger, audio, video, history, overlay)
    , m_index(index)
    , m_controller(controller)
    , m_realtime_inference_utilization(
        new ThreadPoolUtilizationStat(
            GlobalThreadPools::realtime_inference(),
            "Real-Time Pool"
        )
    )
    , m_normal_inference_utilization(
        new ThreadPoolUtilizationStat(
            GlobalThreadPools::normal_inference(),
            "Normal Pool"
        )
    )
    , m_thread_utilization(
        new ThreadUtilizationStat(
            current_thread_handle(),
            "Program Thread:"
        )
    )
{
    overlay.add_stat(*m_realtime_inference_utilization);
    overlay.add_stat(*m_normal_inference_utilization);
    overlay.add_stat(*m_thread_utilization);
}





}
}
