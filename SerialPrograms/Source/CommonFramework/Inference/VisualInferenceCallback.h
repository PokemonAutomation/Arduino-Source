/*  Visual Inference Callback
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_VisualInferenceCallback_H
#define PokemonAutomation_CommonFramework_VisualInferenceCallback_H

#include <chrono>
#include <vector>
#include <deque>
#include <QImage>
#include "CommonFramework/Tools/VideoFeed.h"

namespace PokemonAutomation{


class VisualInferenceCallback{
public:
    virtual void make_overlays(VideoOverlaySet& items) const = 0;

    //  Return true if the inference session should stop.
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) = 0;
};




}
#endif
