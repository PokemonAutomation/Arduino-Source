/*  Summary Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_SummaryDetector_H
#define PokemonAutomation_PokemonFRLG_SummaryDetector_H

#include <chrono>
#include <atomic>
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

// Detect the default (first) Pokemon Summary page
// This has nature and OT, but no stats
// This will work with all languages, as the page indicators are in the same positions
class SummaryDetector : public StaticScreenDetector{
public:
    SummaryDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_page_1_box;
    ImageFloatBox m_page_2_box;
    ImageFloatBox m_page_3_box;
};
class SummaryWatcher : public DetectorToFinder<SummaryDetector>{
public:
    SummaryWatcher(Color color)
        : DetectorToFinder("SummaryWatcher", std::chrono::milliseconds(250), color)
    {}
};

// Detect that we are on page 2 of a pokemon summary
// This is the stats screen
// Note that when switching pages, wait a bit before pressing left/right
class SummaryPage2Detector : public StaticScreenDetector{
public:
    SummaryPage2Detector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_page_1_box;
    ImageFloatBox m_page_2_box;
    ImageFloatBox m_page_3_box;
};
class SummaryPage2Watcher : public DetectorToFinder<SummaryPage2Detector>{
public:
    SummaryPage2Watcher(Color color)
        : DetectorToFinder("SummaryPage2Watcher", std::chrono::milliseconds(250), color)
    {}
};

// Detect that we are on page 3 of a pokemon summary
// This contains the pokemon's moves
class SummaryPage3Detector : public StaticScreenDetector{
public:
    SummaryPage3Detector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_page_1_box;
    ImageFloatBox m_page_2_box;
    ImageFloatBox m_page_3_box;
};
class SummaryPage3Watcher : public DetectorToFinder<SummaryPage3Detector>{
public:
    SummaryPage3Watcher(Color color)
        : DetectorToFinder("SummaryPage3Watcher", std::chrono::milliseconds(250), color)
    {}
};

}
}
}

#endif
