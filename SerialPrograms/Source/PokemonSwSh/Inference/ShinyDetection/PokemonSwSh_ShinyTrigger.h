/*  Shiny Trigger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyTrigger_H
#define PokemonAutomation_PokemonSwSh_ShinyTrigger_H

#include <vector>
#include <QImage>
#include "Kernels/Waterfill/Kernels_Waterfill.h"
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



struct SparkleSet{
    std::vector<ImagePixelBox> balls;
    std::vector<ImagePixelBox> stars;
    std::vector<ImagePixelBox> squares;
    std::vector<ImagePixelBox> lines;

    double alpha_overall() const;
    void draw_boxes(
        VideoOverlaySet& overlays,
        const QImage& frame,
        const ImageFloatBox& inference_box
    ) const;
};

SparkleSet find_sparkles(Kernels::PackedBinaryMatrix& matrix);
SparkleSet find_sparkles(const QImage& image);





#if 0
class PerFrameSparkleTracker{
public:



private:
//    std::multimap<double, ImagePixelBox, std::greater<double>> m_balls;
//    std::multimap<double, ImagePixelBox, std::greater<double>> m_stars;
//    std::multimap<double, ImagePixelBox, std::greater<double>> m_squares;
//    std::multimap<double, ImagePixelBox, std::greater<double>> m_lines;
};
#endif



}
}
}
#endif
