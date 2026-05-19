/*  BDSP AutoStory - Overworld Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Detects that the player is in the free-roaming overworld state:
 *  no dialog box, no battle menu, and no in-game menu open.
 *
 *  BDSP does not ship a standalone OverworldDetector in PokemonBDSP/Inference/,
 *  so this composite detector is defined here for AutoStory use.
 *
 *  Reuses:
 *    - ShortDialogDetector   (PokemonBDSP_DialogDetector.h) — absence required
 *    - BattleMenuDetector    (PokemonBDSP_BattleMenuDetector.h) — absence required
 *    - MenuDetector          (PokemonBDSP_MenuDetector.h) — absence required
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_AutoStory_OverworldDetector_H
#define PokemonAutomation_PokemonBDSP_AutoStory_OverworldDetector_H

#include "Common/Cpp/Color.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_MenuDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


//  Static (single-frame) version — use when you already have a frame.
class OverworldDetector : public StaticScreenDetector{
public:
    OverworldDetector(Color color = COLOR_CYAN);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  Returns true when none of: short dialog, battle menu, or in-game menu
    //  are present on the given frame.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ShortDialogDetector  m_dialog;
    BattleMenuDetector   m_battle_menu;
    MenuDetector         m_menu;
};


//  Watcher (callback) version — use with run_until<> or wait_until().
class OverworldWatcher : public OverworldDetector, public VisualInferenceCallback{
public:
    OverworldWatcher(Color color = COLOR_CYAN);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;
};


}
}
}
#endif
