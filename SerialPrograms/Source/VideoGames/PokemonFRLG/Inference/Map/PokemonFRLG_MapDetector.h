/*  Map Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_MapDetector_H
#define PokemonAutomation_PokemonFRLG_MapDetector_H

#include <chrono>
#include "Common/Cpp/Color.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
    class CancellableScope;
    class VideoFeed;
namespace NintendoSwitch{
namespace PokemonFRLG{

// Detect the map screen for Kanto
class KantoMapDetector : public StaticScreenDetector{
public:
    KantoMapDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_top_box;
    ImageFloatBox m_left_box;
    ImageFloatBox m_right_box;
    ImageFloatBox m_land_box;
};
class KantoMapWatcher : public DetectorToFinder<KantoMapDetector>{
public:
    KantoMapWatcher(Color color)
        : DetectorToFinder("KantoMapWatcher", std::chrono::milliseconds(250), color)
    {}
};

// Detect the Fly map of any region: Kanto or the Sevii Islands. Checks the
// blue title bar and the white side borders. (KantoMapDetector also checks
// for Kanto's land, so it misses the Sevii Islands maps.)
class FlyMapDetector : public StaticScreenDetector{
public:
    FlyMapDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_top_box;
    ImageFloatBox m_left_box;
    ImageFloatBox m_right_box;
};
class FlyMapWatcher : public DetectorToFinder<FlyMapDetector>{
public:
    FlyMapWatcher(Color color)
        : DetectorToFinder("FlyMapWatcher", std::chrono::milliseconds(250), color)
    {}
};



}
}
}

#endif
