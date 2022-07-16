/*  MMO Sprite Star Symbol Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Detect the star symbols on MMO sprite on the map
 */

#ifndef PokemonAutomation_PokemonLA_MMOSpriteStarSymbolDetector_H
#define PokemonAutomation_PokemonLA_MMOSpriteStarSymbolDetector_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"

#include "CommonFramework/ImageTools/FloatPixel.h"
#include "CommonFramework/ImageTypes/ImageReference.h"

#include <vector>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

// Detect the star symbols on MMO sprite on the map
class MMOSpriteStarSymbolDetector : public VisualInferenceCallback{
public:
    MMOSpriteStarSymbolDetector(const ImageViewRGB32& frame, const std::vector<ImagePixelBox>& star_boxes);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  Return true if the inference session should stop.
    virtual bool process_frame(const QImage& frame, WallClock timestamp) override;

    // Not thread safe: must call after inference session ends.
    bool is_star(size_t index) const { return m_is_star[index]; }

private:
    const std::vector<ImagePixelBox>& m_boxes;
    std::vector<bool> m_is_star;
    std::vector<double> m_rmsd;
    std::vector<FloatPixel> m_symbol_colors;
    size_t m_num_frames = 0;
    size_t m_frame_width = 0;
    size_t m_frame_height = 0;
    std::vector<ImageViewRGB32> m_initial_images;

};


}
}
}
#endif
