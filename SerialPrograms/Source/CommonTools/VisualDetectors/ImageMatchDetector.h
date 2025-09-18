/*  Image Match Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_ImageMatchDetector_H
#define PokemonAutomation_CommonTools_ImageMatchDetector_H

#include <memory>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/FloatPixel.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{

class ImageMatchDetector : public StaticScreenDetector{
public:
    ImageMatchDetector(
        std::shared_ptr<const ImageRGB32> reference_image, const ImageFloatBox& box,
        double max_rmsd, bool scale_brightness = false,
        Color color = COLOR_RED
    );

    double rmsd(const ImageViewRGB32& frame) const;

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    std::shared_ptr<const ImageRGB32> m_reference_image;
    ImageViewRGB32 m_reference_image_cropped;
    FloatPixel m_average_brightness;

    double m_max_rmsd;
    bool m_scale_brightness;

    Color m_color;
    ImageFloatBox m_box;
};


class ImageMatchWatcher : public ImageMatchDetector, public VisualInferenceCallback{
public:
    ImageMatchWatcher(
        std::shared_ptr<const ImageRGB32> reference_image, const ImageFloatBox& box,
        double max_rmsd, bool scale_brightness = false,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(0),
        Color color = COLOR_RED
    );

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    std::chrono::milliseconds m_hold_duration;

    bool m_last_match;
    WallClock m_start_of_match;
};




}
#endif
