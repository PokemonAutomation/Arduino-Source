/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonRSE_DialogDetector_H
#define PokemonAutomation_PokemonRSE_DialogDetector_H

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
namespace PokemonRSE{

// Battle dialog boxes are teal
/*
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
*/

// Battle menu is a white box on the right
// Teal dialog box remains on the left
// For R/S, the selection arrow is only in JPN ver, other languages use a box
// Positions slightly different on non-JPN Emerald but all langs use an arrow
class BattleMenuDetector : public StaticScreenDetector{
public:
    BattleMenuDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    //R/S both JPN and ENG, E for JPN
    ImageFloatBox m_menu_top_box;
    ImageFloatBox m_menu_bottom_box;
    ImageFloatBox m_dialog_top_box;
    ImageFloatBox m_dialog_right_box;

    //Emerald for non-JPN
    ImageFloatBox m_menu_top_eme_box;
    ImageFloatBox m_menu_right_eme_box;
    ImageFloatBox m_dialog_top_eme_box;
    ImageFloatBox m_dialog_right_eme_box;
};
class BattleMenuWatcher : public DetectorToFinder<BattleMenuDetector>{
public:
    BattleMenuWatcher(Color color)
        : DetectorToFinder("BattleMenuWatcher", std::chrono::milliseconds(250), color)
    {}
};



// Detect the red advancement arrow by filtering for red.
// This is the same as BattleDialogDetector apart from the arrow
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

// Future note: when given a choice popup, there is no advance arrow.


}
}
}

#endif
