/*  ML Image Display Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This class holds the run-time state of an image display session
 *
 *  This class is fully thread-safe. You can call any functions from anywhere at
 *  anytime.
 *
 *  Warning: Constructing this class requires an "option" parameter. It is not
 *  safe to modify this "option" parameter during the lifetime of this class.
 *
 */

#ifndef ML_ImageDisplaySession_H
#define ML_ImageDisplaySession_H

#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/AudioPipeline/AudioSession.h"
#include "CommonFramework/VideoPipeline/VideoSession.h"
#include "CommonFramework/VideoPipeline/VideoOverlaySession.h"
#include "CommonFramework/Recording/StreamHistorySession.h"
#include "Controllers/ControllerSession.h"
#include "Integrations/ProgramTrackerInterfaces.h"
#include "ML_ImageDisplayOption.h"

namespace PokemonAutomation{
    class CpuUtilizationStat;
    class ThreadUtilizationStat;
namespace ML{




class ImageDisplaySession {
public:
    ~ImageDisplaySession();
    ImageDisplaySession(
        ImageDisplayOption& option,
        uint64_t program_id,
        size_t console_number
    );

public:
    Logger& logger(){ return m_logger; }
    virtual VideoFeed& video() { return m_video; }
    VideoOverlay& overlay(){ return m_overlay; }

public:
    void get(ImageDisplayOption& option);
    void set(const ImageDisplayOption& option);

    VideoSession& video_session(){ return m_video; }
    VideoOverlaySession& overlay_session(){ return m_overlay; }

private:
    TaggedLogger m_logger;

    VideoSession m_video;
    VideoOverlaySession m_overlay;

    std::unique_ptr<CpuUtilizationStat> m_cpu_utilization;
    std::unique_ptr<ThreadUtilizationStat> m_main_thread_utilization;
};



}
}
#endif
