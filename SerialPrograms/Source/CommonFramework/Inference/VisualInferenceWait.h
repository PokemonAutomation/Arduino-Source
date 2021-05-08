/*  Visual Inference Wait
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      Wait for an inference detection.
 *
 */

#ifndef PokemonAutomation_CommonFramework_VisualInferenceWait_H
#define PokemonAutomation_CommonFramework_VisualInferenceWait_H

#include <functional>
#include <vector>
#include <set>
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "VisualInferenceCallback.h"

namespace PokemonAutomation{


class VisualInferenceWait{
public:
    VisualInferenceWait(
        ProgramEnvironment& env,
        VideoFeed& feed,
        std::chrono::milliseconds timeout,
        std::chrono::milliseconds period = std::chrono::milliseconds(50)
    );

    void operator+=(std::function<bool(const QImage&)>&& callback);
    void operator+=(VisualInferenceCallback& callback);

    //  Run inference and wait for result. Returns false if timed out.
    bool run();

private:
    ProgramEnvironment& m_env;
    VideoFeed& m_feed;
    std::chrono::milliseconds m_timeout;
    std::chrono::milliseconds m_period;
    std::vector<std::function<bool(const QImage&)>> m_callbacks0;
    std::set<VisualInferenceCallback*> m_callbacks1;
};




}
#endif
