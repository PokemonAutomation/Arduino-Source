/*  Shiny Trigger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_ShinyTrigger_H
#define PokemonAutomation_PokemonBDSP_ShinyTrigger_H

#include <vector>
#include <deque>
#include <QImage>
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Inference/ImageTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


struct ShinyImageAlpha{
    double shiny = 0;
    double star = 0;

    void max(const ShinyImageAlpha& x);
};

struct ShinyImageDetection{
    std::vector<ImagePixelBox> balls;
    std::vector<ImagePixelBox> stars;

    void accumulate(
        const QImage& image, uint64_t frame_counter,
        Logger* logger = nullptr
    );

    ShinyImageAlpha alpha() const;
    void add_overlays(
        std::deque<InferenceBoxScope>& overlays, VideoOverlay& overlay,
        const QImage& screen, const ImageFloatBox& box
    ) const;
};






}
}
}
#endif
