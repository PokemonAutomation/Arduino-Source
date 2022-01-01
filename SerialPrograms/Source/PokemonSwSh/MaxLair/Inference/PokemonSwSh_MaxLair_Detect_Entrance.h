/*  Max Lair Detect Entrance
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_Entrance_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_Entrance_H

#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


class EntranceDetector : public VisualInferenceCallback{
public:
    EntranceDetector(const QImage& entrance_screen);

    bool detect(const QImage& screen);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    ImageFloatBox m_box0;
    QImage m_entrance_screen;

};



}
}
}
}
#endif
