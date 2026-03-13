/*  Battle Dialog Detectors
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_BattleDialogs_H
#define PokemonAutomation_PokemonFRLG_BattleDialogs_H

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




// Battle dialog boxes are teal, similar to r/s/e
class BattleDialogDetector : public StaticScreenDetector{
public:
    BattleDialogDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_dialog_top_box;
    ImageFloatBox m_dialog_right_box;

    ImageFloatBox m_dialog_top_jpn_box;
    ImageFloatBox m_dialog_right_jpn_box;
};
class BattleDialogWatcher : public DetectorToFinder<BattleDialogDetector>{
public:
    BattleDialogWatcher(Color color)
        : DetectorToFinder("BattleDialogWatcher", std::chrono::milliseconds(250), color)
    {}
};


// Battle menu is a white box on the right with FIGHT/POKEMON/BAG/RUN
// The dialog box with "what will POKEMON do?" is dark teal/navy
class BattleMenuDetector : public StaticScreenDetector{
public:
    BattleMenuDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_menu_top_box;
    ImageFloatBox m_menu_right_box;
    ImageFloatBox m_dialog_top_box;
    ImageFloatBox m_dialog_right_box;

    ImageFloatBox m_menu_top_jpn_box;
    ImageFloatBox m_menu_right_jpn_box;
    ImageFloatBox m_dialog_top_jpn_box;
    ImageFloatBox m_dialog_right_jpn_box;
};
class BattleMenuWatcher : public DetectorToFinder<BattleMenuDetector>{
public:
    BattleMenuWatcher(Color color)
        : DetectorToFinder("BattleMenuWatcher", std::chrono::milliseconds(250), color)
    {}
};


// Detect the red advancement arrow by filtering for red.
// No red colored text in battle?
// reuse the battledialog checks for the top and right
class AdvanceBattleDialogDetector : public StaticScreenDetector{
public:
    AdvanceBattleDialogDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_dialog_box;
    ImageFloatBox m_dialog_top_box;
    ImageFloatBox m_dialog_right_box;

    ImageFloatBox m_dialog_jpn_box;
    ImageFloatBox m_dialog_top_jpn_box;
    ImageFloatBox m_dialog_right_jpn_box;
};
class AdvanceBattleDialogWatcher : public DetectorToFinder<AdvanceBattleDialogDetector>{
public:
    AdvanceBattleDialogWatcher(Color color)
        : DetectorToFinder("AdvanceBattleDialogWatcher", std::chrono::milliseconds(250), color)
    {}
};



}
}
}

#endif
