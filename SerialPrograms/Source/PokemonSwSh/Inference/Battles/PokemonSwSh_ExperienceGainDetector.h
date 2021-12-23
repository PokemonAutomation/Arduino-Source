/*  Battle Won Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BattleWonDetector_H
#define PokemonAutomation_PokemonSwSh_BattleWonDetector_H

#include "Common/Cpp/FixedLimitVector.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"
#include "PokemonSwSh_BattleDialogDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ExperienceGainDetector : public StaticScreenDetector{
public:
    ExperienceGainDetector(QColor color = Qt::red);

    virtual void make_overlays(OverlaySet& items) const override;
    virtual bool detect(const QImage& screen) const override;

private:
    QColor m_color;
    BattleDialogDetector m_dialog;
    FixedLimitVector<std::pair<ImageFloatBox, ImageFloatBox>> m_rows;
};


class ExperienceGainWatcher : public ExperienceGainDetector, public VisualInferenceCallback{
public:
    using ExperienceGainDetector::ExperienceGainDetector;

    virtual void make_overlays(OverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;
};


}
}
}
#endif
