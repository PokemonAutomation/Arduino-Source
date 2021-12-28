/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BattleDialogDetector_H
#define PokemonAutomation_PokemonBDSP_BattleDialogDetector_H

#include "CommonFramework/Inference/VisualDetector.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class ShortDialogDetector : public StaticScreenDetector{
public:
    ShortDialogDetector(QColor color = Qt::red);

    virtual void make_overlays(OverlaySet& items) const override;
    virtual bool detect(const QImage& screen) const override;

private:
    QColor m_color;
//    ImageFloatBox m_bottom;
    ImageFloatBox m_left_white;
    ImageFloatBox m_left;
    ImageFloatBox m_right_white;
    ImageFloatBox m_right;
};
class ShortDialogWatcher : public ShortDialogDetector, public VisualInferenceCallback{
public:
    using ShortDialogDetector::ShortDialogDetector;

    virtual void make_overlays(OverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;
};




class BattleDialogDetector : public StaticScreenDetector{
public:
    BattleDialogDetector(QColor color = Qt::red);

    virtual void make_overlays(OverlaySet& items) const override;
    virtual bool detect(const QImage& screen) const override;

private:
    QColor m_color;
    ImageFloatBox m_bottom;
    ImageFloatBox m_left_white;
    ImageFloatBox m_left;
    ImageFloatBox m_right;
};



}
}
}
#endif
