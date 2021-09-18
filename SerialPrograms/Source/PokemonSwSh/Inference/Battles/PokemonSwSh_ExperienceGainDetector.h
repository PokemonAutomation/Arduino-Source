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
#include "CommonFramework/Inference/VisualInferenceCallback.h"
#include "PokemonSwSh_BattleDialogDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ExperienceGainDetector : public VisualInferenceCallback{
public:
    ExperienceGainDetector(VideoOverlay& overlay);

    bool detect(const QImage& screen) const;

    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    BattleDialogDetector m_dialog;
    FixedLimitVector<std::pair<ImageFloatBox, ImageFloatBox>> m_rows;
};


}
}
}
#endif
