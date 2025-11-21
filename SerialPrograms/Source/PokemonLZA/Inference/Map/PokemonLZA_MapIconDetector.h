/*  Map Icon Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_MapIconDetector_H
#define PokemonAutomation_PokemonLZA_MapIconDetector_H

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/DetectedBoxes.h"
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

const char* map_icon_type_to_string(MapIconType type);
MapIconType string_to_map_icon_type(const std::string& str);




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

    const std::vector<DetectedBox>& last_detected();

    virtual void reset_state() override {
        m_last_detected.clear();
        m_last_detected_box.clear();
    }

private:
    Color m_color;
    const MapIconMatcher& m_matcher;
    ImageFloatBox m_box;
    VideoOverlay* m_overlay;

    std::vector<DetectedBox> m_last_detected;
    std::deque<OverlayBoxScope> m_last_detected_box;
};





}
}
}
#endif
