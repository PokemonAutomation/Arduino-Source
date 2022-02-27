/*  Overworld Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_OverworldDetector_H
#define PokemonAutomation_PokemonLA_OverworldDetector_H

#include "PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.h"
#include "PokemonLA_MountDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class OverworldDetector : public VisualInferenceCallback{
public:
    OverworldDetector(LoggerQt& logger, VideoOverlay& overlay)
        : VisualInferenceCallback("OverworldDetector")
        , m_arc_phone(logger, overlay, std::chrono::milliseconds(100), true)
    {}

    virtual void make_overlays(VideoOverlaySet& items) const override{
        m_arc_phone.make_overlays(items);
        m_mount.make_overlays(items);
    }
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override{
        m_arc_phone.process_frame(frame, timestamp);
        if (!m_arc_phone.detected()){
            return false;
        }
        return m_mount.detect(frame) != MountState::NOTHING;
    }

private:
    ArcPhoneDetector m_arc_phone;
    MountDetector m_mount;
};



}
}
}
#endif
