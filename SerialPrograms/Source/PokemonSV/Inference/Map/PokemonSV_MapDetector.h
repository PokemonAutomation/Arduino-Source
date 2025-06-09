/*  Map Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_MapDetector_H
#define PokemonAutomation_PokemonSV_MapDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


// The area on the screen that the program can clearly read map info, when the map is opened.
extern ImageFloatBox MAP_READABLE_AREA;

// Detect the orange arrow pointing up on the lower left corner of the map, when the map is in the fixed view mode.
// This arrow is used to differentate the rotation view mode.
class MapFixedViewDetector : public StaticScreenDetector{
public:
    MapFixedViewDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_arrow_box;
};
class MapFixedViewWatcher : public DetectorToFinder<MapFixedViewDetector>{
public:
    MapFixedViewWatcher(Color color = COLOR_RED)
         : DetectorToFinder("MapFixedViewWatcher", std::chrono::milliseconds(1000), color)
    {}
};


// Detect the orange double-direction arrows pointing sideways on the lower left corner of the map, when the map is
// in the rotated view mode.
// This arrow is used to differentate the fixed view mode.
class MapRotatedViewDetector : public StaticScreenDetector{
public:
    MapRotatedViewDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_arrow_box;
};
class MapRotatedViewWatcher : public DetectorToFinder<MapRotatedViewDetector>{
public:
    MapRotatedViewWatcher(Color color = COLOR_RED)
         : DetectorToFinder("MapRotatedViewWatcher", std::chrono::milliseconds(1000), color)
    {}
};

// Detect whether map is opened and ready to be closed.
// Can also detect whether the map is in the fixed view or rotated view mode.
class MapWatcher : public VisualInferenceCallback{
public:
    MapWatcher(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

    // Whether the map is in the fixed view instead of rotated view.
    bool map_in_fixed_view() const { return m_in_fixed_view; }

private:
    WhiteButtonWatcher  m_exit_watcher;
    MapFixedViewWatcher m_fixed_view_watcher;
    MapRotatedViewWatcher m_rotated_view_watcher;

    bool m_in_fixed_view = false;
};


}
}
}
#endif
