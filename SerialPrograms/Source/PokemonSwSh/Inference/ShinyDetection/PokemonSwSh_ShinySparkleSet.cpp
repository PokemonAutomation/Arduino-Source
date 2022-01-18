/*  Shiny Sparkle Set
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/BinaryImage/BinaryImage_FilterRgb32.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "PokemonSwSh_SparkleDetectorRadial.h"
#include "PokemonSwSh_SparkleDetectorSquare.h"
#include "PokemonSwSh_ShinySparkleSet.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Kernels;
using namespace Kernels::Waterfill;


ShinySparkleSetSwSh::~ShinySparkleSetSwSh(){}
void ShinySparkleSetSwSh::clear(){
    balls.clear();
    stars.clear();
    squares.clear();
    lines.clear();
    m_alpha_overall = 0;
    m_alpha_star = 0;
    m_alpha_square = 0;
}

std::string ShinySparkleSetSwSh::to_str() const{
    std::string str;
    if (m_alpha_overall < 1.0){
        return str;
    }
    str += "SparkleDetector";
    if (!balls.empty()){
        str += " - Balls: " + std::to_string(balls.size());
    }
    if (!stars.empty()){
        str += " - Stars: " + std::to_string(stars.size());
    }
    if (!squares.empty()){
        str += " - Squares: " + std::to_string(squares.size());
    }
    if (!lines.empty()){
        str += " - Lines: " + std::to_string(lines.size());
    }
    str += " - (alpha = ";
    str += std::to_string(m_alpha_overall);
    str += ")";
    return str;
}
void ShinySparkleSetSwSh::draw_boxes(
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
void ShinySparkleSetSwSh::update_alphas(){
    double ball_alpha   = 0.4 * balls.size();
    double star_alpha   = 0.5 * stars.size();
    double square_alpha = 0.3 * squares.size();
    double line_alpha   = 0.4 * lines.size();
    m_alpha_overall = ball_alpha + star_alpha + square_alpha + line_alpha;
    m_alpha_star = star_alpha;
    m_alpha_square = square_alpha + line_alpha;
}




ShinySparkleSetSwSh find_sparkles(Kernels::PackedBinaryMatrix& matrix){
    ShinySparkleSetSwSh sparkles;
    WaterFillIterator finder(matrix, 20);
    WaterFillObject object;
    while (finder.find_next(object)){
        RadialSparkleDetector radial_sparkle(object);
        if (radial_sparkle.is_ball()){
            sparkles.balls.emplace_back(object.min_x, object.min_y, object.max_x, object.max_y);
            continue;
        }
        if (radial_sparkle.is_star()){
            sparkles.stars.emplace_back(object.min_x, object.min_y, object.max_x, object.max_y);
            continue;
        }
        if (is_line_sparkle(object)){
            sparkles.lines.emplace_back(object.min_x, object.min_y, object.max_x, object.max_y);
            continue;
        }
        if (is_square_sparkle(object)){
            sparkles.squares.emplace_back(object.min_x, object.min_y, object.max_x, object.max_y);
            continue;
        }
    }
    return sparkles;
}
void ShinySparkleSetSwSh::read_from_image(const QImage& image){
    clear();
    if (image.isNull()){
        return;
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
        ShinySparkleSetSwSh sparkles = find_sparkles(matrix[c]);
        sparkles.update_alphas();
        double alpha = sparkles.alpha_overall();
        if (best_alpha < alpha){
            best_alpha = alpha;
            *this = std::move(sparkles);
        }
    }
#else
    for (size_t c = 0; c < 4; c++){
        SparkleSet sparkles = find_sparkles(matrix[c]);
        sparkles.update_alphas();
        set.balls.insert(set.balls.end(), sparkles.balls.begin(), sparkles.balls.end());
        set.stars.insert(set.stars.end(), sparkles.stars.begin(), sparkles.stars.end());
        set.squares.insert(set.squares.end(), sparkles.squares.begin(), sparkles.squares.end());
        set.lines.insert(set.lines.end(), sparkles.lines.begin(), sparkles.lines.end());
    }
#endif
}




void ShinySparkleAggregator::add_frame(const QImage& image, const ShinySparkleSetSwSh& sparkles){
    m_best_overall = std::max(m_best_overall, sparkles.alpha_overall());

    double current_star = sparkles.alpha_star();
    double current_square = sparkles.alpha_square();

    m_best_star = std::max(m_best_star, current_star);
    m_best_square = std::max(m_best_square, current_square);

    double type_alpha = std::max(current_star, current_square);
    if (m_best_type < type_alpha){
        m_best_type = type_alpha;
        m_best_image = image;
    }
}






}
}
}


