/*  Console Handle
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/InferenceInfra/VisualInferencePivot.h"
#include "CommonFramework/InferenceInfra/AudioInferencePivot.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "ConsoleHandle.h"

namespace PokemonAutomation{


ConsoleHandle::ConsoleHandle(ConsoleHandle&& x) = default;
ConsoleHandle::~ConsoleHandle(){
    m_overlay.remove_stat(*m_audio_pivot);
    m_overlay.remove_stat(*m_video_pivot);
}


ConsoleHandle::ConsoleHandle(
    size_t index,
    Logger& logger,
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

void ConsoleHandle::initialize_inference_threads(CancellableScope& scope, AsyncDispatcher& dispatcher){
    m_video_pivot = std::make_unique<VisualInferencePivot>(scope, m_video, dispatcher);
    m_audio_pivot = std::make_unique<AudioInferencePivot>(scope, m_audio, dispatcher);
    m_overlay.add_stat(*m_video_pivot);
    m_overlay.add_stat(*m_audio_pivot);
}




}
