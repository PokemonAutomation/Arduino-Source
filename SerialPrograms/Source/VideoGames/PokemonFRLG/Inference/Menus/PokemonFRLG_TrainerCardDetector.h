/*  Trainer Card Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_MapDetector_H
#define PokemonAutomation_PokemonFRLG_MapDetector_H

#include <chrono>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
    class CancellableScope;
    class VideoFeed;
namespace NintendoSwitch{
namespace PokemonFRLG{

// Detect the map screen for Kanto
class TrainerCardDetector : public StaticScreenDetector{
public:
    TrainerCardDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_box_tid_top;
    ImageFloatBox m_box_tid_right;
    ImageFloatBox m_box_stripe_top;
    ImageFloatBox m_box_stripe_right;
    ImageFloatBox m_box_border_top;
    ImageFloatBox m_box_border_right;
};
class TrainerCardWatcher : public DetectorToFinder<TrainerCardDetector>{
public:
    TrainerCardWatcher(Color color)
        : DetectorToFinder("TrainerCardWatcher", std::chrono::milliseconds(250), color)
    {}
};



}
}
}

#endif
