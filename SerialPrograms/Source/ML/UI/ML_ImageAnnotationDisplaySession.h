/*  ML Image Annotation Display Session
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

#ifndef POKEMON_AUTOMATION_ML_ImageAnnotationDisplaySession_H
#define POKEMON_AUTOMATION_ML_ImageAnnotationDisplaySession_H

#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/AudioPipeline/AudioSession.h"
#include "CommonFramework/VideoPipeline/VideoSession.h"
#include "CommonFramework/VideoPipeline/VideoOverlaySession.h"
#include "CommonFramework/Recording/StreamHistorySession.h"
#include "CommonFramework/VideoPipeline/VideoSources/VideoSource_StillImage.h"
#include "Controllers/ControllerSession.h"
#include "Integrations/ProgramTrackerInterfaces.h"
#include "ML_ImageAnnotationDisplayOption.h"


namespace PokemonAutomation{
    class CpuUtilizationStat;
    class ThreadUtilizationStat;
namespace ML{




class ImageAnnotationDisplaySession {
public:
    ~ImageAnnotationDisplaySession();
    ImageAnnotationDisplaySession(ImageAnnotationDisplayOption& option);

public:
    Logger& logger(){ return m_logger; }
    virtual VideoFeed& video() { return m_video_session; }
    VideoOverlay& overlay(){ return m_overlay; }

    void set_image_source(const std::string& path);

    const ImageAnnotationDisplayOption& option() const { return m_display_option; }

public:

    VideoSession& video_session(){ return m_video_session; }
    VideoOverlaySession& overlay_session(){ return m_overlay; }

private:
    TaggedLogger m_logger;

    ImageAnnotationDisplayOption& m_display_option;
    VideoSourceOption m_still_image_option;
    VideoSession m_video_session;
    VideoOverlaySession m_overlay;

    std::shared_ptr<VideoSourceDescriptor_StillImage> m_still_image_descriptor;

    std::unique_ptr<CpuUtilizationStat> m_cpu_utilization;
    std::unique_ptr<ThreadUtilizationStat> m_main_thread_utilization;
};



}
}
#endif
