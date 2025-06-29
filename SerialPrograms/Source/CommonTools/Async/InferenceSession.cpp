/*  Inference Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/InferenceCallbacks/AudioInferenceCallback.h"
#include "CommonTools/InferencePivots/VisualInferencePivot.h"
#include "CommonTools/InferencePivots/AudioInferencePivot.h"
#include "InferenceSession.h"

namespace PokemonAutomation{


InferenceSession::InferenceSession(
    Cancellable& scope, VideoStream& stream,
    const std::vector<PeriodicInferenceCallback>& callbacks,
    std::chrono::milliseconds default_video_period,
    std::chrono::milliseconds default_audio_period
)
    : m_stream(stream)
    , m_overlays(stream.overlay())
    , m_triggered(nullptr)
{
    try{
        for (size_t c = 0; c < callbacks.size(); c++){
            const PeriodicInferenceCallback& callback = callbacks[c];
            if (callback.callback == nullptr){
                continue;
            }
            if (!m_map.emplace(callback.callback, c).second){
                throw InternalProgramError(&stream.logger(), PA_CURRENT_FUNCTION, "Attempted to add the same callback twice.");
            }
            switch (callback.callback->type()){
            case InferenceType::VISUAL:{
                VisualInferenceCallback& visual_callback = static_cast<VisualInferenceCallback&>(*callback.callback);
                stream.video_inference_pivot().add_callback(
                    scope, &m_triggered,
                    visual_callback,
                    callback.period > std::chrono::milliseconds(0) ? callback.period : default_video_period
                );
                visual_callback.make_overlays(m_overlays);
                break;
            }
            case InferenceType::AUDIO:
                stream.audio_inference_pivot().add_callback(
                    scope, &m_triggered,
                    static_cast<AudioInferenceCallback&>(*callback.callback),
                    callback.period > std::chrono::milliseconds(0) ? callback.period : default_audio_period
                );
                break;
            }
        }
    }catch (...){
        clear();
        throw;
    }
}
InferenceSession::~InferenceSession(){
    clear();
}


InferenceCallback* InferenceSession::triggered_ptr() const{
    return m_triggered.load(std::memory_order_acquire);
}
int InferenceSession::triggered_index() const{
    auto iter = m_map.find(m_triggered.load(std::memory_order_acquire));
    if (iter != m_map.end()){
        return (int)iter->second;
    }
    return -1;
}


void InferenceSession::clear() noexcept{
    const double DIVIDER = (double)(std::chrono::milliseconds(1) / std::chrono::microseconds(1));
    const char* UNITS = " ms";
    for (auto& item : m_map){
        switch (item.first->type()){
        case InferenceType::VISUAL:{
            StatAccumulatorI32 stats = m_stream.video_inference_pivot().remove_callback(static_cast<VisualInferenceCallback&>(*item.first));
            try{
                stats.log(m_stream.logger(), item.first->label(), UNITS, DIVIDER);
            }catch (...){}
            break;
        }
        case InferenceType::AUDIO:{
            StatAccumulatorI32 stats = m_stream.audio_inference_pivot().remove_callback(static_cast<AudioInferenceCallback&>(*item.first));
            try{
                stats.log(m_stream.logger(), item.first->label(), UNITS, DIVIDER);
            }catch (...){}
            break;
        }
        }
    }
}




}
