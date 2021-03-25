/*  Shiny Trigger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyTrigger_H
#define PokemonAutomation_PokemonSwSh_ShinyTrigger_H

#include <vector>
#include <QImage>
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Inference/ImageTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


struct ShinyImageDetection{
    std::vector<PixelBox> balls;
    std::vector<PixelBox> stars;
    std::vector<PixelBox> squares;
    std::vector<PixelBox> lines;

    void accumulate(
        const QImage& image, uint64_t frame_counter,
        Logger* logger = nullptr
    );


    double alpha() const;
};






}
}
}
#endif
