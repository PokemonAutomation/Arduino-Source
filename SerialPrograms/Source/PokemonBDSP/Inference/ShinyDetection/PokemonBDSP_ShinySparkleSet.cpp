/*  Shiny Sparkle Set
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_SparkleDetectorRadial.h"
#include "PokemonBDSP_ShinySparkleSet.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Kernels;
using namespace Kernels::Waterfill;


ShinySparkleSetBDSP::~ShinySparkleSetBDSP(){}
void ShinySparkleSetBDSP::clear(){
    balls.clear();
    stars.clear();
    m_alpha_overall = 0;
}

ShinySparkleSetBDSP ShinySparkleSetBDSP::extract_subbox(const ImagePixelBox& subbox) const{
    ShinySparkleSetBDSP ret;
    for (const ImagePixelBox& box : balls){
        if (subbox.min_x <= box.min_x &&
            subbox.min_y <= box.min_y &&
            subbox.max_x >= box.max_x &&
            subbox.max_y >= box.max_y
        ){
            ret.balls.emplace_back(box);
        }
    }
    for (const ImagePixelBox& box : stars){
        if (subbox.min_x <= box.min_x &&
            subbox.min_y <= box.min_y &&
            subbox.max_x >= box.max_x &&
            subbox.max_y >= box.max_y
        ){
            ret.stars.emplace_back(box);
        }
    }
    ret.update_alphas();
    return ret;
}
std::string ShinySparkleSetBDSP::to_str() const{
    std::string str;
    if (m_alpha_overall < 3.0){
        return str;
    }
    str += "SparkleDetector";
    if (!balls.empty()){
        str += " - Balls: " + std::to_string(balls.size());
    }
    if (!stars.empty()){
        str += " - Stars: " + std::to_string(stars.size());
    }
    str += " - (alpha = " + tostr_default(m_alpha_overall) + ")";
    return str;
}
void ShinySparkleSetBDSP::draw_boxes(
    VideoOverlaySet& overlays,
    const ImageViewRGB32& frame,
    const ImageFloatBox& inference_box
) const{
    for (const ImagePixelBox& box : balls){
        overlays.add(COLOR_GREEN, translate_to_parent(frame, inference_box, box));
    }
    for (const ImagePixelBox& box : stars){
        overlays.add(COLOR_BLUE, translate_to_parent(frame, inference_box, box));
    }
}
void ShinySparkleSetBDSP::update_alphas(){
    double ball_alpha   = 1.0 * balls.size();
    double star_alpha   = 1.0 * stars.size();
    m_alpha_overall = ball_alpha + star_alpha;
}




ShinySparkleSetBDSP find_sparkles(WaterfillSession& session){
    ShinySparkleSetBDSP sparkles;
    auto finder = session.make_iterator(20);
    WaterfillObject object;
    while (finder->find_next(object, true)){
        PokemonSwSh::RadialSparkleDetector radial_sparkle(object);
        if (radial_sparkle.is_ball()){
            sparkles.balls.emplace_back(object.min_x, object.min_y, object.max_x, object.max_y);
            continue;
        }
        if (radial_sparkle.is_star()){
            sparkles.stars.emplace_back(object.min_x, object.min_y, object.max_x, object.max_y);
            continue;
        }
    }
    return sparkles;
}
void ShinySparkleSetBDSP::read_from_image(const ImageViewRGB32& image){
    clear();
    if (!image){
        return;
    }

    std::vector<PackedBinaryMatrix> matrices = compress_rgb32_to_binary_range(
        image,
        {
            {0xff606000, 0xffffffff},
            {0xff707000, 0xffffffff},
            {0xff808000, 0xffffffff},
            {0xff909000, 0xffffffff},
        }
    );
    auto session = make_WaterfillSession();

    double best_alpha = 0;
    for (PackedBinaryMatrix& matrix : matrices){
        session->set_source(matrix);
        ShinySparkleSetBDSP sparkles = find_sparkles(*session);
        sparkles.update_alphas();
        double alpha = sparkles.alpha_overall();
        if (best_alpha < alpha){
            best_alpha = alpha;
            *this = std::move(sparkles);
        }
    }
}




void ShinySparkleAggregator::add_frame(const std::shared_ptr<const ImageRGB32>& image, const ShinySparkleSetBDSP& sparkles){
    double alpha = sparkles.alpha_overall();
    if (m_best_overall < alpha){
        m_best_overall = alpha;
        m_best_image = image;
    }
}





}
}
}
