/*  Max Lair Detect Item Select Menu
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_ItemSelectMenu_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_ItemSelectMenu_H

#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


class ItemSelectDetector : public VisualInferenceCallback{
public:
    ItemSelectDetector(bool stop_no_detect);

    bool detect(const QImage& screen) const;

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const QImage& frame, WallClock timestamp) override final;


private:
    bool m_stop_on_no_detect;
    ImageFloatBox m_bottom_main;
    ImageFloatBox m_bottom_right;
    ImageFloatBox m_blue;
};


}
}
}
}
#endif
