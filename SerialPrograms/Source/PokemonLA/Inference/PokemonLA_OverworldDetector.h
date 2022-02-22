/*  Overworld Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_OverworldDetector_H
#define PokemonAutomation_PokemonLA_OverworldDetector_H

#include "PokemonLA_MountDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class OverworldDetector : public VisualInferenceCallback{
public:
    OverworldDetector()
        : VisualInferenceCallback("OverworldDetector")
    {}

    virtual void make_overlays(VideoOverlaySet& items) const override{
        m_detector.make_overlays(items);
    }
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override{
        return m_detector.detect(frame) != MountState::NOTHING;
    }

private:
    MountDetector m_detector;
};



}
}
}
#endif
