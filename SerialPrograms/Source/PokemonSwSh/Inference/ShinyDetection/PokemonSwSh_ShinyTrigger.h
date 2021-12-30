/*  Shiny Trigger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyTrigger_H
#define PokemonAutomation_PokemonSwSh_ShinyTrigger_H

#include <vector>
#include <QImage>
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Inference/ImageTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


struct ShinyImageAlpha{
    double shiny = 0;
    double star = 0;
    double square = 0;

    void max(const ShinyImageAlpha& x);
};

struct ShinyImageDetection{
    std::vector<ImagePixelBox> balls;
    std::vector<ImagePixelBox> stars;
    std::vector<ImagePixelBox> squares;
    std::vector<ImagePixelBox> lines;

    void accumulate(
        const QImage& image, uint64_t frame_counter,
        Logger* logger = nullptr
    );


    ShinyImageAlpha alpha() const;
};






}
}
}
#endif
