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
#include "PokemonSwSh_SquareTrigger.h"
#include "PokemonSwSh_ShinyTrigger.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

void ShinyImageDetection::accumulate(
    const QImage& image, uint64_t frame_counter,
    Logger* logger
){
    FillMatrix matrix(image);

    BrightYellowLightFilter filter;
    matrix.apply_filter(image, filter);

    std::vector<FillGeometry> objects;
    objects = find_all_objects(matrix, true);

    QImage debug = image;
    for (int r = 0; r < image.height(); r++){
        for (int c = 0; c < image.width(); c++){
            if (matrix[r][c] == 0){
                debug.setPixel(c, r, 0);
            }
        }
    }

//    cout << "objects = " << objects.size() << endl;

//    bool detection = false;
    for (FillGeometry& object : objects){
//        this->balls.emplace_back(object.box);

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
            str += "ShinyDetector Star: [{" + QString::number(object.center_x) + ", " + QString::number(object.center_y);
            str += "}, " + QString::number(object.area) + "]";
            if (logger){
                logger->log(str, "purple");
            }
        }
#endif
#if 1
        if (is_square(matrix, object)){
//            detection = true;
            this->squares.emplace_back(object.box);
            QString str;
            str += "ShinyDetector Square: [{" + QString::number(object.center_x) + ", " + QString::number(object.center_y);
            str += "}, " + QString::number(object.area) + "]";
            if (logger){
                logger->log(str, "purple");
            }
        }
#endif
#if 1
        if (is_square_beam(matrix, object, 0.5)){
//            detection = true;
            this->lines.emplace_back(object.box);
            QString str;
            str += "ShinyDetector Line: [{" + QString::number(object.box.width());
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
    if (detection){
//        cout << "objects = " << objects.size() << endl;
        image.save("test-" + QString::number(frame_counter) + "-O.png");
        debug.save("test-" + QString::number(frame_counter) + "-P.png");
    }
#endif
//    debug.save("test.png");

}



double ShinyImageDetection::alpha() const{
    double x = 1.0;
    x *= std::pow(3.0, balls.size());
    x *= std::pow(3.0, stars.size());
    x *= std::pow(1.2, squares.size());
    x *= std::pow(5.0, lines.size());
    return x;
}







}
}
}
