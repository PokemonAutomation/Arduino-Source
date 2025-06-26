/*  Shiny Sparkle Set
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_ShinySparkleSet_H
#define PokemonAutomation_Pokemon_ShinySparkleSet_H

#include <string>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
    class Logger;
namespace Pokemon{


class ShinySparkleSet{
public:
    virtual ~ShinySparkleSet() = default;
    virtual void clear() = 0;

    virtual std::string to_str() const = 0;

    virtual void read_from_image(size_t screen_area, const ImageViewRGB32& image) = 0;
    virtual void draw_boxes(
        VideoOverlaySet& overlays,
        const ImageViewRGB32& frame,
        const ImageFloatBox& inference_box
    ) const = 0;

};



class ShinySparkleTracker : public VisualInferenceCallback{
public:
    ShinySparkleTracker(
        Logger& logger, VideoOverlay& overlay,
        ShinySparkleSet& sparkle_set,
        const ImageFloatBox& box
    );

    void clear_boxes();
    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;
    using VisualInferenceCallback::process_frame;

private:
    ImageFloatBox m_box;
    Logger& m_logger;
    ShinySparkleSet& m_current_sparkles;
    VideoOverlaySet m_overlays;
};





}
}
#endif
