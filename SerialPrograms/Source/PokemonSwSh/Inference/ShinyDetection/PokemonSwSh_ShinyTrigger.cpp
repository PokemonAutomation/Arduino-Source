/*  Shiny Trigger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "Common/Compiler.h"
#include "CommonFramework/BinaryImage/BinaryImage_FilterRgb32.h"
#include "CommonFramework/ImageTools/FillGeometry.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "PokemonSwSh_ShinyFilters.h"
#include "PokemonSwSh_SparkleTrigger.h"
#include "PokemonSwSh_SquareDetector.h"
#include "PokemonSwSh_SquareTrigger.h"
#include "PokemonSwSh_ShinyTrigger.h"
#include "PokemonSwSh_SparkleDetectorRadial.h"
#include "PokemonSwSh_SparkleDetectorSquare.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Kernels;
using namespace Kernels::Waterfill;


void ShinyImageAlpha::max(const ShinyImageAlpha& x){
    shiny = std::max(shiny, x.shiny);
    star = std::max(star, x.star);
    square = std::max(square, x.square);
}


void ShinyImageDetection::accumulate(
    const QImage& image, uint64_t frame_counter,
    Logger* logger
){
//    auto time0 = std::chrono::system_clock::now();
    if (image.isNull()){
        return;
    }
//    auto time1 = std::chrono::system_clock::now();
//    cout << "Image Check: " << std::chrono::duration_cast<std::chrono::milliseconds>(time1 - time0).count() << endl;

//    auto time0 = std::chrono::system_clock::now();
    CellMatrix matrix(image);
//    auto time1 = std::chrono::system_clock::now();
//    cout << "Matrix: " << std::chrono::duration_cast<std::chrono::milliseconds>(time1 - time0).count() << endl;

//    auto time0 = std::chrono::system_clock::now();
    BrightYellowLightFilter filter;
    matrix.apply_filter(image, filter);
//    auto time1 = std::chrono::system_clock::now();
//    cout << "Filter: " << std::chrono::duration_cast<std::chrono::milliseconds>(time1 - time0).count() << endl;

    if (filter.count * 2 > (size_t)image.width() * image.height()){
        return;
    }

//    auto time0 = std::chrono::system_clock::now();
    std::vector<FillGeometry> objects;
    objects = find_all_objects(matrix, 1, true);
//    auto time1 = std::chrono::system_clock::now();
//    cout << "FillGeometry: " << std::chrono::duration_cast<std::chrono::milliseconds>(time1 - time0).count() << endl;

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

//    auto time2 = std::chrono::system_clock::now();
//    bool detection = false;
//    int c = 0;
    for (FillGeometry& object : objects){
//        this->balls.emplace_back(object.box);

//        if (c++ != 4){
//            continue;
//        }

//        image.copy(object.box.min_x, object.box.min_y, object.box.width(), object.box.height()).save("test-" + QString::number(c++) + ".png");

        SparkleDetectorOld detector(matrix, object);
#if 1
//        auto time4 = std::chrono::system_clock::now();
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
//        auto time5 = std::chrono::system_clock::now();
//        cout << std::chrono::duration_cast<std::chrono::milliseconds>(time4 - time5).count() << endl;
#endif
#if 1
//        auto time6 = std::chrono::system_clock::now();
        if (is_square2(image, matrix, object)){
//            detection = true;
            this->squares.emplace_back(object.box);
            QString str;
            str += "ShinyDetector: Square [{" + QString::number(object.center_x()) + ", " + QString::number(object.center_y());
            str += "}, " + QString::number(object.area) + "]";
            if (logger){
                logger->log(str, COLOR_PURPLE);
            }
        }
//        auto time7 = std::chrono::system_clock::now();
//        cout << std::chrono::duration_cast<std::chrono::milliseconds>(time6 - time7).count() << endl;
//        if (time1 - time0 > std::chrono::milliseconds(20)){
//            image.save("slow-inference.png");
//        }
#endif
#if 1
//        auto time0 = std::chrono::system_clock::now();
        if (is_square_beam(matrix, object, 0.5)){
//            detection = true;
            this->lines.emplace_back(object.box);
            QString str;
            str += "ShinyDetector: Line [{" + QString::number(object.box.width());
            str += " : {" + QString::number(object.center_x()) + ", " + QString::number(object.center_y());
            str += "}, " + QString::number(object.area) + "]";
            if (logger){
                logger->log(str, COLOR_PURPLE);
            }
        }
//        auto time1 = std::chrono::system_clock::now();
//        cout << std::chrono::duration_cast<std::chrono::milliseconds>(time1 - time0).count() << endl;
#endif
    }
//    auto time3 = std::chrono::system_clock::now();
//    cout << "Objects: " << std::chrono::duration_cast<std::chrono::milliseconds>(time3 - time2).count() << endl;
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






double SparkleSet::alpha_overall() const{
    double ball_alpha   = 0.4 * balls.size();
    double star_alpha   = 0.4 * stars.size();
    double square_alpha = 0.3 * squares.size();
    double line_alpha   = 0.5 * lines.size();
    return ball_alpha + star_alpha + square_alpha + line_alpha;
}
void SparkleSet::draw_boxes(
    VideoOverlaySet& overlays,
    const QImage& frame,
    const ImageFloatBox& inference_box
) const{
    for (const ImagePixelBox& box : balls){
        overlays.add(COLOR_GREEN, translate_to_parent(frame, inference_box, box));
    }
    for (const ImagePixelBox& box : stars){
        overlays.add(COLOR_BLUE, translate_to_parent(frame, inference_box, box));
    }
    for (const ImagePixelBox& box : squares){
        overlays.add(COLOR_MAGENTA, translate_to_parent(frame, inference_box, box));
    }
    for (const ImagePixelBox& box : lines){
        overlays.add(COLOR_YELLOW, translate_to_parent(frame, inference_box, box));
    }
}


SparkleSet find_sparkles(Kernels::PackedBinaryMatrix& matrix){
    SparkleSet set;
    WaterFillIterator finder(matrix, 20);
    WaterFillObject object;
    while (finder.find_next(object)){
        RadialSparkleDetector radial_sparkle(object);
        if (radial_sparkle.is_ball()){
            set.balls.emplace_back(object.min_x, object.min_y, object.max_x, object.max_y);
            continue;
        }
        if (radial_sparkle.is_star()){
            set.stars.emplace_back(object.min_x, object.min_y, object.max_x, object.max_y);
            continue;
        }
        if (is_line_sparkle(object)){
            set.lines.emplace_back(object.min_x, object.min_y, object.max_x, object.max_y);
            continue;
        }
        if (is_square_sparkle(object)){
            set.squares.emplace_back(object.min_x, object.min_y, object.max_x, object.max_y);
            continue;
        }
    }
    return set;
}
SparkleSet find_sparkles(const QImage& image){
    SparkleSet set;
    if (image.isNull()){
        return set;
    }

    Kernels::PackedBinaryMatrix matrix[4];
//    matrix0 = compress_rgb32_to_binary_min(image, 160, 160, 0);
//    matrix1 = compress_rgb32_to_binary_min(image, 176, 176, 0);
//    matrix2 = compress_rgb32_to_binary_min(image, 192, 192, 0);
//    matrix3 = compress_rgb32_to_binary_min(image, 208, 208, 0);
    compress4_rgb32_to_binary_range(
        image,
        matrix[0], 0xffa0a000, 0xffffffff,
        matrix[1], 0xffb0b000, 0xffffffff,
        matrix[2], 0xffc0c000, 0xffffffff,
        matrix[3], 0xffd0d000, 0xffffffff
    );

#if 1
    double best_alpha = 0;
    for (size_t c = 0; c < 4; c++){
        SparkleSet sparkles = find_sparkles(matrix[c]);
        double alpha = sparkles.alpha_overall();
        if (best_alpha < alpha){
            best_alpha = alpha;
            set = std::move(sparkles);
        }
    }
#else
    for (size_t c = 0; c < 4; c++){
        SparkleSet sparkles = find_sparkles(matrix[c]);
        set.balls.insert(set.balls.end(), sparkles.balls.begin(), sparkles.balls.end());
        set.stars.insert(set.stars.end(), sparkles.stars.begin(), sparkles.stars.end());
        set.squares.insert(set.squares.end(), sparkles.squares.begin(), sparkles.squares.end());
        set.lines.insert(set.lines.end(), sparkles.lines.begin(), sparkles.lines.end());
    }
#endif

    return set;
}










}
}
}
