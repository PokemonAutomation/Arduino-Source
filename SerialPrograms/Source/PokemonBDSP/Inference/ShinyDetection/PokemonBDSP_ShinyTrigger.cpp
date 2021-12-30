/*  Shiny Trigger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "CommonFramework/ImageTools/FillGeometry.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyFilters.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_SparkleTrigger.h"
#include "PokemonBDSP_ShinyTrigger.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



struct BrightYellowLightFilter{
    size_t count = 0;

    void operator()(CellMatrix::ObjectID& cell, const QImage& image, int x, int y){
        QRgb pixel = image.pixel(x, y);
//        int set = qRed(pixel) > 160 && qGreen(pixel) > 160 && qBlue(pixel) > 128;
        int set = (pixel & 0x00808080) == 0x00808080 ? 1 : 0;
        cell = set;
        count += set;
    }
};




void ShinyImageAlpha::max(const ShinyImageAlpha& x){
    shiny = std::max(shiny, x.shiny);
    star = std::max(star, x.star);
}



void ShinyImageDetection::accumulate(
    const QImage& image, uint64_t frame_counter,
    Logger* logger
){
    if (image.isNull()){
        return;
    }

    CellMatrix matrix(image);

    BrightYellowLightFilter filter;
    matrix.apply_filter(image, filter);

    if (filter.count * 2 > (size_t)image.width() * image.height()){
        return;
    }

    std::vector<FillGeometry> objects;
    objects = find_all_objects(matrix, 1, true);

//    size_t count = 0;
    for (FillGeometry& object : objects){

        PokemonSwSh::SparkleDetector detector(matrix, object);
        if (detector.is_ball()){
//            detection = true;
            this->balls.emplace_back(object.box);
            QString str;
            str += "ShinyDetector: Ball [{" + QString::number(object.center_x()) + ", " + QString::number(object.center_y());
            str += "}, " + QString::number(object.area) + "]";
            if (logger){
                logger->log(str, COLOR_PURPLE);
            }
        }
        if (detector.is_star()){
//            detection = true;
            this->stars.emplace_back(object.box);
            QString str;
            str += "ShinyDetector: Star [{" + QString::number(object.center_x()) + ", " + QString::number(object.center_y());
            str += "}, " + QString::number(object.area) + "]";
            if (logger){
                logger->log(str, COLOR_PURPLE);
            }
        }
//        extract_box(image, object.box).save("test-" + QString::number(count++) + ".png");
    }

#if 0
    if (!this->squares.empty()){
        image.save("detection-square-" + QString::number(frame_counter) + "-O.png");
    }
    if (!this->lines.empty()){
        image.save("detection-line-" + QString::number(frame_counter) + "-O.png");
    }
#endif
//    debug.save("test.png");

}



ShinyImageAlpha ShinyImageDetection::alpha() const{
    double ball_alpha   = 1.0 * balls.size();
    double star_alpha   = 1.0 * stars.size();

    ShinyImageAlpha alpha;
    alpha.shiny = ball_alpha + star_alpha;
    alpha.star = star_alpha;
    return alpha;
}
void ShinyImageDetection::add_overlays(
    std::deque<InferenceBoxScope>& overlays, VideoOverlay& overlay,
    const QImage& screen, const ImageFloatBox& inference_box
) const{
    for (const auto& item : balls){
        ImageFloatBox box = translate_to_parent(screen, inference_box, item);
        overlays.emplace_back(overlay, box, COLOR_GREEN);
    }
    for (const auto& item : stars){
        ImageFloatBox box = translate_to_parent(screen, inference_box, item);
        overlays.emplace_back(overlay, box, COLOR_GREEN);
    }
}




}
}
}
