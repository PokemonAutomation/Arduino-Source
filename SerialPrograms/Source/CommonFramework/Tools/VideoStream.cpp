/*  Video Stream
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Containers/Pimpl.tpp"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/Recording/StreamHistorySession.h"
#include "CommonTools/InferencePivots/VisualInferencePivot.h"
#include "CommonTools/InferencePivots/AudioInferencePivot.h"
#include "VideoStream.h"

namespace PokemonAutomation{



VideoStream::VideoStream(VideoStream&& x) = default;
VideoStream::~VideoStream(){
    m_overlay.remove_stat(*m_audio_pivot);
    m_overlay.remove_stat(*m_video_pivot);
}
VideoStream::VideoStream(
    Logger& logger,
    AudioFeed& audio,
    VideoFeed& video,
    const StreamHistorySession& history,
    VideoOverlay& overlay
)
    : m_logger(logger)
    , m_audio(audio)
    , m_video(video)
    , m_history(history)
    , m_overlay(overlay)
{}


bool VideoStream::save_stream_history(const std::string& filename){
    return history().save(filename);
}


void VideoStream::initialize_inference_threads(CancellableScope& scope, AsyncDispatcher& dispatcher){
    m_video_pivot.reset(scope, m_video, dispatcher);
    m_audio_pivot.reset(scope, m_audio, dispatcher);
    m_overlay.add_stat(*m_video_pivot);
    m_overlay.add_stat(*m_audio_pivot);
}



}
