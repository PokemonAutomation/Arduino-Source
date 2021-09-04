/*  Raid Catch Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_RaidCatchDetector_H
#define PokemonAutomation_PokemonSwSh_RaidCatchDetector_H

#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class RaidCatchDetector : public VisualInferenceCallbackWithCommandStop{
public:
    RaidCatchDetector(VideoOverlay& overlay);

    bool detect(const QImage& screen);
    bool wait(
        ProgramEnvironment& env,
        VideoFeed& feed,
        std::chrono::milliseconds timeout
    );

    virtual bool on_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    bool has_timed_out(std::chrono::milliseconds timeout) const;


private:
    InferenceBoxScope m_left0;
    InferenceBoxScope m_right0;
    InferenceBoxScope m_left1;
    InferenceBoxScope m_right1;
    InferenceBoxScope m_text0;
    InferenceBoxScope m_text1;
    std::chrono::time_point<std::chrono::system_clock> m_start_time;
};


}
}
}
#endif

