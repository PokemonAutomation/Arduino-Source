/*  Start Battle Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_StartBattleDetector_H
#define PokemonAutomation_PokemonSwSh_StartBattleDetector_H

#include "CommonFramework/Inference/VisualInferenceCallback.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleDialogDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



class StartBattleWatcher : public VisualInferenceCallback{
public:
    StartBattleWatcher(QColor color = Qt::red);

    virtual void make_overlays(OverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;

private:
    QColor m_color;
    ImageFloatBox m_screen_box;
    BattleDialogDetector m_dialog;
};







}
}
}
#endif

