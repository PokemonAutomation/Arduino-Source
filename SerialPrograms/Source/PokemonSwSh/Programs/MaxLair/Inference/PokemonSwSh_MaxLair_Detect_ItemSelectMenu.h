/*  Max Lair Detect Item Select Menu
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_ItemSelectMenu_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_ItemSelectMenu_H

#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


class ItemSelectDetector : public VisualInferenceCallbackWithCommandStop{
public:
    ItemSelectDetector(VideoOverlay& overlay, bool stop_no_detect);

    bool detect(const QImage& screen) const;

    virtual bool on_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    bool m_stop_on_no_detect;
    InferenceBoxScope m_bottom_main;
    InferenceBoxScope m_bottom_right;
    InferenceBoxScope m_blue;
};


}
}
}
}
#endif
