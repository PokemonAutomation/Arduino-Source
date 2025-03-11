/*  MMO Sprite Star Symbol Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Detect the star symbols on MMO sprite on the map
 */

#ifndef PokemonAutomation_PokemonLA_MMOSpriteStarSymbolDetector_H
#define PokemonAutomation_PokemonLA_MMOSpriteStarSymbolDetector_H

#include <vector>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/FloatPixel.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

// Detect the star symbols on MMO sprite on the map
class MMOSpriteStarSymbolDetector : public VisualInferenceCallback{
public:
    MMOSpriteStarSymbolDetector(const ImageViewRGB32& frame, const std::vector<ImagePixelBox>& star_boxes);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  Return true if the inference session should stop.
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

    // Get detection result: whether the `index`-th sprite has a star symbol. 
    // Not thread safe: must call after inference session ends.
    bool is_star(size_t index) const { return m_is_star[index]; }

    // Get the rmsd value used to determine if `index`-th sprite has an animating symbol.
    // If the value is larger than a threshold, then it is considered the sprite has an animating symbol.
    // This is for debugging purposes.
    // Not thread safe: must call after inference session ends.
    double animation_value(size_t index) const { return m_rmsd[index]; }
    
    // Get the color used to determine if `index`-th sprite has a star symbol.
    // Not thread safe: must call after inference session ends.
    const FloatPixel& symbol_color(size_t index) const { return m_symbol_colors[index]; }

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
