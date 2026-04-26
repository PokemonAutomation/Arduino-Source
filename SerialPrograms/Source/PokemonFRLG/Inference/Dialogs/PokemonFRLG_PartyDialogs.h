/*  Party Dialog Detectors
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_PartyDialogs_H
#define PokemonAutomation_PokemonFRLG_PartyDialogs_H

#include <chrono>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
    class CancellableScope;
    class VideoFeed;
namespace NintendoSwitch{
namespace PokemonFRLG{


// The Party menu has a white box in the bottom right corner when a Pokemon is selected
// The background around the edges is dark teal/navy
class PartySelectionDetector : public StaticScreenDetector{
public:
    PartySelectionDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_dialog_right_box;
    ImageFloatBox m_page_background_box;
};
class PartySelectionWatcher : public DetectorToFinder<PartySelectionDetector>{
public:
    PartySelectionWatcher(Color color)
        : DetectorToFinder("PartySelectionWatcher", std::chrono::milliseconds(250), color)
    {}
};

enum class PartyLevelUpDialog{
    plus,
    stats,
    either
};

class PartyLevelUpDetector : public StaticScreenDetector{
public:
    PartyLevelUpDetector(Color color, PartyLevelUpDialog dialog_type);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    PartyLevelUpDialog dialog_type;
    ImageFloatBox m_border_top_box;
    ImageFloatBox m_border_right_box;
    ImageFloatBox m_dialog_top_box;
    ImageFloatBox m_dialog_right_box;
    ImageFloatBox m_plus_box;
};
class PartyLevelUpWatcher : public DetectorToFinder<PartyLevelUpDetector>{
public:
    PartyLevelUpWatcher(Color color, PartyLevelUpDialog dialog_type)
        : DetectorToFinder("BattleLevelUpWatcher", std::chrono::milliseconds(250), color, dialog_type)
    {}
};


class PartyMoveLearnDetector : public StaticScreenDetector{
public:
    PartyMoveLearnDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_border_top_box;
    ImageFloatBox m_border_right_box;
    ImageFloatBox m_dialog_top_box;
    ImageFloatBox m_dialog_right_box;
};
class PartyMoveLearnWatcher : public DetectorToFinder<PartyMoveLearnDetector>{
public:
    PartyMoveLearnWatcher(Color color)
        : DetectorToFinder("PartyMoveLearnWatcher", std::chrono::milliseconds(250), color)
    {}
};



}
}
}

#endif
