/*  Map Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_MapDetector_H
#define PokemonAutomation_PokemonBDSP_MapDetector_H

#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class MapDetector : public VisualInferenceCallback{
public:
    MapDetector();

    bool detect(const QImage& frame) const;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;

private:
    ImageFloatBox m_box0;
    ImageFloatBox m_box1;
    ImageFloatBox m_box2;
    ImageFloatBox m_box3;
};


}
}
}
#endif
