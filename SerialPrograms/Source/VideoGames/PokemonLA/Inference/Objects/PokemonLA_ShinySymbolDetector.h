/*  Shiny Symbol Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_ShinySymbolDetector_H
#define PokemonAutomation_PokemonLA_ShinySymbolDetector_H

#include <vector>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


extern const ImageFloatBox SHINY_SYMBOL_BOX_BOTTOM;


std::vector<ImagePixelBox> find_shiny_symbols(const ImageViewRGB32& image);


class ShinySymbolWatcher : public VisualInferenceCallback{
public:
    ShinySymbolWatcher(VideoOverlay& overlay, const ImageFloatBox& box = {0, 0, 1, 1});

    bool detected() const{ return !m_matches.empty(); }
    const std::vector<ImagePixelBox>& matches() const{ return m_matches; }

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  Return true if the inference session should stop.
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    ImageFloatBox m_box;
    std::vector<ImagePixelBox> m_matches;
    VideoOverlaySet m_overlays;
};

class ShinySymbolWaiter : public ShinySymbolWatcher{
public:
    using ShinySymbolWatcher::ShinySymbolWatcher;

    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;
};




}
}
}
#endif
