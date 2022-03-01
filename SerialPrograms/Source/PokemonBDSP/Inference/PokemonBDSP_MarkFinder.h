/*  Mark Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_MarkFinder_H
#define PokemonAutomation_PokemonBDSP_MarkFinder_H

#include <vector>
#include <deque>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


std::vector<ImagePixelBox> find_exclamation_marks(const QImage& image);



class MarkTracker : public VisualInferenceCallback{
public:
    MarkTracker(VideoOverlay& overlay, const ImageFloatBox& box);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;

protected:
    VideoOverlay& m_overlay;
    ImageFloatBox m_box;
    std::deque<InferenceBoxScope> m_marks;
};

class MarkDetector : public MarkTracker{
public:
    using MarkTracker::MarkTracker;

    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;
};



}
}
}
#endif
