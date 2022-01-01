/*  Y-Comm Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_YCommDetector_H
#define PokemonAutomation_PokemonSwSh_YCommDetector_H

#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class YCommMenuDetector : public VisualInferenceCallback{
public:
    YCommMenuDetector(bool is_on);

    bool detect(const QImage& screen);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;

private:
    bool m_is_on;
    ImageFloatBox m_top;
    ImageFloatBox m_bottom;
};



}
}
}
#endif
