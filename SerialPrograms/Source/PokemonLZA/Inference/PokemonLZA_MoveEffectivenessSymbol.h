/*  Move Effectiveness Symbol
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_MoveEffectivenessSymbol_H
#define PokemonAutomation_PokemonLZA_MoveEffectivenessSymbol_H

#include <optional>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{




enum class MoveEffectivenessSymbol{
    None,
    NoEffect,
    NotVeryEffective,
    Normal,
    SuperEffective,
};



class MoveEffectivenessSymbolMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    MoveEffectivenessSymbolMatcher(MoveEffectivenessSymbol symbol);

    static const MoveEffectivenessSymbolMatcher& NoEffect(){
        static MoveEffectivenessSymbolMatcher matcher(MoveEffectivenessSymbol::NoEffect);
        return matcher;
    }
    static const MoveEffectivenessSymbolMatcher& NotVeryEffective(){
        static MoveEffectivenessSymbolMatcher matcher(MoveEffectivenessSymbol::NotVeryEffective);
        return matcher;
    }
    static const MoveEffectivenessSymbolMatcher& Normal(){
        static MoveEffectivenessSymbolMatcher matcher(MoveEffectivenessSymbol::Normal);
        return matcher;
    }
    static const MoveEffectivenessSymbolMatcher& SuperEffective(){
        static MoveEffectivenessSymbolMatcher matcher(MoveEffectivenessSymbol::SuperEffective);
        return matcher;
    }

    virtual bool check_image(Resolution input_resolution, const ImageViewRGB32& image) const override{
        size_t min_width = 40 * input_resolution.width / 3840;
        size_t min_height = 40 * input_resolution.height / 2160;
        return image.width() >= min_width && image.height() >= min_height;
    };


};





class MoveEffectivenessSymbolDetector : public StaticScreenDetector{
public:
    MoveEffectivenessSymbolDetector(Color color, VideoOverlay* overlay);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;


private:
    bool detect_box(
        int index,
        const ImageViewRGB32& screen,
        const ImageFloatBox& box
    );


private:
    Color m_color;
    const ImageFloatBox m_box_top;
    const ImageFloatBox m_box_left;
    const ImageFloatBox m_box_right;
    const ImageFloatBox m_box_bottom;
    VideoOverlay* m_overlay;

    MoveEffectivenessSymbol m_last_detection[4];
    std::optional<OverlayBoxScope> m_last_detection_boxes[4];
};
class MoveEffectivenessSymbolWatcher : public DetectorToFinder<MoveEffectivenessSymbolDetector>{
public:
    MoveEffectivenessSymbolWatcher(
        Color color,
        VideoOverlay* overlay = nullptr,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("MoveEffectivenessSymbolWatcher", hold_duration, color, overlay)
    {}
};






}
}
}
#endif
