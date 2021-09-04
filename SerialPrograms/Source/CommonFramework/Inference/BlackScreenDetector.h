/*  Black Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *
 *      Returns true after a black screen has been detected and
 * the black screen has ended.
 *
 */

#ifndef PokemonAutomation_CommonFramework_BlackScreenDetector_H
#define PokemonAutomation_CommonFramework_BlackScreenDetector_H

#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{


class BlackScreenDetector : public VisualInferenceCallbackWithCommandStop{
public:
    BlackScreenDetector(VideoOverlay& overlay);
    BlackScreenDetector(VideoOverlay& overlay, const ImageFloatBox& box);

    bool black_is_over(const QImage& frame);
    virtual bool on_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
   ) override;

private:
    InferenceBoxScope m_box;
    bool m_has_been_black;
};



}
#endif
