/*  Shiny Trigger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "CommonFramework/Inference/FillMatrix.h"
#include "CommonFramework/Inference/FillGeometry.h"
#include "PokemonSwSh_ShinyFilters.h"
#include "PokemonSwSh_SparkleTrigger.h"
#include "PokemonSwSh_SquareDetector.h"
#include "PokemonSwSh_SquareTrigger.h"
#include "PokemonSwSh_ShinyTrigger.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void ShinyImageAlpha::max(const ShinyImageAlpha& x){
    shiny = std::max(shiny, x.shiny);
    star = std::max(star, x.star);
    square = std::max(square, x.square);
}


void ShinyImageDetection::accumulate(
    const QImage& image, uint64_t frame_counter,
    Logger* logger
){
    FillMatrix matrix(image);

    BrightYellowLightFilter filter;
    matrix.apply_filter(image, filter);

    if (filter.count * 2 > (size_t)image.width() * image.height()){
        return;
    }

    std::vector<FillGeometry> objects;
    objects = find_all_objects(matrix, 1, true);

#if 0
    QImage debug = image;
    for (int r = 0; r < image.height(); r++){
        for (int c = 0; c < image.width(); c++){
            if (matrix[r][c] == 0){
                debug.setPixel(c, r, 0);
            }
        }
    }
#endif

//    cout << "objects = " << objects.size() << endl;

//    bool detection = false;
//    int c = 0;
    for (FillGeometry& object : objects){
//        this->balls.emplace_back(object.box);

//        if (c++ != 4){
//            continue;
//        }

//        image.copy(object.box.min_x, object.box.min_y, object.box.width(), object.box.height()).save("test-" + QString::number(c++) + ".png");

        SparkleDetector detector(matrix, object);
#if 1
        if (detector.is_ball()){
//            detection = true;
            this->balls.emplace_back(object.box);
            QString str;
            str += "ShinyDetector: Ball [{" + QString::number(object.center_x) + ", " + QString::number(object.center_y);
            str += "}, " + QString::number(object.area) + "]";
            if (logger){
                logger->log(str, "purple");
            }
        }
        if (detector.is_star()){
//            detection = true;
            this->stars.emplace_back(object.box);
            QString str;
            str += "ShinyDetector: Star [{" + QString::number(object.center_x) + ", " + QString::number(object.center_y);
            str += "}, " + QString::number(object.area) + "]";
            if (logger){
                logger->log(str, "purple");
            }
        }
#endif
#if 1
//        auto time0 = std::chrono::system_clock::now();
        if (is_square2(image, matrix, object)){
//            detection = true;
            this->squares.emplace_back(object.box);
            QString str;
            str += "ShinyDetector: Square [{" + QString::number(object.center_x) + ", " + QString::number(object.center_y);
            str += "}, " + QString::number(object.area) + "]";
            if (logger){
                logger->log(str, "purple");
            }
        }
//        auto time1 = std::chrono::system_clock::now();
//        if (time1 - time0 > std::chrono::milliseconds(20)){
//            image.save("slow-inference.png");
//        }
#endif
#if 1
        if (is_square_beam(matrix, object, 0.5)){
//            detection = true;
            this->lines.emplace_back(object.box);
            QString str;
            str += "ShinyDetector: Line [{" + QString::number(object.box.width());
            str += " : {" + QString::number(object.center_x) + ", " + QString::number(object.center_y);
            str += "}, " + QString::number(object.area) + "]";
            if (logger){
                logger->log(str, "purple");
            }
        }
#endif
    }
//    cout << "objects = " << objects.size() << endl;

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
    double ball_alpha   = 0.4 * balls.size();
    double star_alpha   = 0.4 * stars.size();
    double square_alpha = 0.3 * squares.size();
    double line_alpha   = 0.5 * lines.size();

    ShinyImageAlpha alpha;
    alpha.shiny = ball_alpha + star_alpha + square_alpha + line_alpha;
    alpha.star = star_alpha;
    alpha.square = square_alpha + line_alpha;
    return alpha;
}







}
}
}
