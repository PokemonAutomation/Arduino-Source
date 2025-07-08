/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BattleDialogDetector_H
#define PokemonAutomation_PokemonBDSP_BattleDialogDetector_H

#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"
#include "PokemonBDSP_SelectionArrow.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

// Detect short dialog boxes that are used in all kinds of ingame situations where texts are displayed.
// The only place the long dialog boxes appear is during pokemon battles. But after battle, the exp gain
// text is in a short dialog box.
class ShortDialogDetector : public StaticScreenDetector{
public:
    ShortDialogDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
//    ImageFloatBox m_bottom;
    ImageFloatBox m_left_white;
    ImageFloatBox m_left;
    ImageFloatBox m_right_white;
    ImageFloatBox m_right;
};
class ShortDialogWatcher : public DetectorToFinder<ShortDialogDetector>{
public:
    ShortDialogWatcher(Color color = COLOR_RED)
        : DetectorToFinder("ShortDialogWatcher", std::chrono::milliseconds(250), color)
    {}
};



// Detect the long dialog boxes that only appear during pokemon battles.
// Note after battle, the exp gain text is in a short dialog box.
class BattleDialogDetector : public StaticScreenDetector{
public:
    BattleDialogDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_bottom;
    ImageFloatBox m_left_white;
    ImageFloatBox m_left;
    ImageFloatBox m_right;
};


class ShortDialogPromptDetector : public VisualInferenceCallback{
public:
    ShortDialogPromptDetector(
        VideoOverlay& overlay,
        const ImageFloatBox& box,
        Color color = COLOR_RED
    );

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    ShortDialogDetector m_dialog;
    SelectionArrowFinder m_arrow;
};




}
}
}
#endif
