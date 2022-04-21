/*  Max Lair Detect Entrance
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_Entrance_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_Entrance_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


class EntranceDetector : public VisualInferenceCallback{
public:
    EntranceDetector(const QImage& entrance_screen);

    bool detect(const QImage& screen);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const QImage& frame, WallClock timestamp) override final;


private:
    ImageFloatBox m_box0;
    QImage m_entrance_screen;

};



}
}
}
}
#endif
