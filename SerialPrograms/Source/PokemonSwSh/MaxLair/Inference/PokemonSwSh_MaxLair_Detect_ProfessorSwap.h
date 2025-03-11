/*  Max Lair Detect Professor Swap
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_ProfessorSwap_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_ProfessorSwap_H

#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


class ProfessorSwapDetector : public SelectionArrowFinder{
public:
    ProfessorSwapDetector(VideoOverlay& overlay, bool enable);

    bool detect(const ImageViewRGB32& screen);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override final;


private:
    bool m_enabled;
//    ImageFloatBox m_dialog0;
//    ImageFloatBox m_dialog1;
    ImageFloatBox m_bottom_main;
};





}
}
}
}
#endif
