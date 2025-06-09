/*  Map Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_MapMenuDetector_H
#define PokemonAutomation_PokemonSV_MapMenuDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{

class VideoOverlaySet;
class VideoOverlay;
class OverlayBoxScope;

namespace NintendoSwitch{
namespace PokemonSV{


// Detect whether the "Fly here" menu is opened on map
// Note: it detects the white background of the menu. So if the underlying map is pure white, it will have false positives.
class MapFlyMenuDetector : public StaticScreenDetector{
public:
    MapFlyMenuDetector(Color color);
    virtual ~MapFlyMenuDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

protected:
    Color m_color;
    ImageFloatBox m_middle_box, m_bottom_box;
};


// Watch for the "Fly here" menu to open on map
class MapFlyMenuWatcher : public VisualInferenceCallback{
public:
    MapFlyMenuWatcher(Color color);
    virtual ~MapFlyMenuWatcher();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

protected:
    MapFlyMenuDetector m_detector;
};


// Detect whether the "Set as destination" menu is opened on map.
// This menu has no "Fly here" menuitem.
// Note: it detects the white background of the menu. So if the underlying map is pure white, it will have false positives.
class MapDestinationMenuDetector : public StaticScreenDetector{
public:
    MapDestinationMenuDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

protected:
    Color m_color;
    ImageFloatBox m_bottom_box;
    ImageFloatBox m_fly_menu_box;
};


// Watch for the "Set as destination" menu to open on map.
// This menu has no "Fly here" menuitem.
class MapDestinationMenuWatcher : public VisualInferenceCallback{
public:
    ~MapDestinationMenuWatcher();
    MapDestinationMenuWatcher(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

protected:
    MapDestinationMenuDetector m_detector;
};


}
}
}
#endif
