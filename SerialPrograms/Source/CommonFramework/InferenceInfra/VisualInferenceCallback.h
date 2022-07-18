/*  Visual Inference Callback
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_VisualInferenceCallback_H
#define PokemonAutomation_CommonFramework_VisualInferenceCallback_H

#include <memory>
#include <string>
#include "Common/Compiler.h"
#include "Common/Cpp/Time.h"
#include "InferenceCallback.h"

class QImage;

namespace PokemonAutomation{

class ImageViewRGB32;
class ImageRGB32;
class VideoOverlaySet;


class VisualInferenceCallback : public InferenceCallback{
public:
    VisualInferenceCallback(std::string label)
        : InferenceCallback(InferenceType::VISUAL, std::move(label))
    {}

    virtual void make_overlays(VideoOverlaySet& items) const = 0;

    //  Return true if the inference session should stop.
    //  You must override at least one of these.
    virtual bool process_frame(const QImage& frame, WallClock timestamp);   //  Deprecated
    virtual bool process_frame(const std::shared_ptr<ImageRGB32>& frame, WallClock timestamp);
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp);

};




}
#endif
