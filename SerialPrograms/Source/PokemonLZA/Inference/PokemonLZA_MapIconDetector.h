/*  Map Icon Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_MapIconDetector_H
#define PokemonAutomation_PokemonLZA_MapIconDetector_H

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonLZA{

class MapIconMatcher;


enum class MapIconType{
    PokemonCenter,
    Building,
    BuildingFlyable,
    CafeFlyable,
    Clothing,
    Haircut,
    Shopping,
    WildZone,
    WildZoneFlyable,
    BattleZone,
};




class MapIconDetector : public StaticScreenDetector{
public:
    MapIconDetector(
        Color color,
        MapIconType button_type,
        const ImageFloatBox& box,
        VideoOverlay* overlay = nullptr
    );
    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    const MapIconMatcher& m_matcher;
    ImageFloatBox m_box;
    VideoOverlay* m_overlay;

    struct Detection{
        const std::string& name;
        ImageFloatBox box;
    };

    std::vector<Detection> m_last_detected;
    std::deque<OverlayBoxScope> m_last_detected_box;
};





}
}
}
#endif
