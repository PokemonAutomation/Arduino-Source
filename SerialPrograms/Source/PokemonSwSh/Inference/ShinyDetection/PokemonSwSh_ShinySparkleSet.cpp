/*  Shiny Sparkle Set
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <sstream>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
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
    std::ostringstream ss;
    if (m_alpha_overall < 1.0){
        return "";
    }
    ss << "SparkleDetector";
    if (!balls.empty()){
        ss << " - Balls: " << balls.size();
    }
    if (!stars.empty()){
        ss << " - Stars: " << stars.size();
    }
    if (!squares.empty()){
        ss << " - Squares: " << squares.size();
    }
    if (!lines.empty()){
        ss << " - Lines: " << lines.size();
    }
    ss << " - (alpha = " << m_alpha_overall << ")";
    return ss.str();
}
void ShinySparkleSetSwSh::draw_boxes(
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
    for (const ImagePixelBox& box : squares){
        overlays.add(COLOR_MAGENTA, translate_to_parent(frame, inference_box, box));
    }
    for (const ImagePixelBox& box : lines){
        overlays.add(COLOR_YELLOW, translate_to_parent(frame, inference_box, box));
    }
}
void ShinySparkleSetSwSh::update_alphas(){
    const GameSettings& settings = GameSettings::instance();
    double ball_alpha   = settings.BALL_SPARKLE_ALPHA * balls.size();
    double star_alpha   = settings.STAR_SPARKLE_ALPHA * stars.size();
    double square_alpha = settings.SQUARE_SPARKLE_ALPHA * squares.size();
    double line_alpha   = std::min(settings.LINE_SPARKLE_ALPHA * lines.size(), square_alpha);
    m_alpha_overall = ball_alpha + star_alpha + square_alpha + line_alpha;
    m_alpha_star = star_alpha;
    m_alpha_square = square_alpha + line_alpha;
}




ShinySparkleSetSwSh find_sparkles(size_t screen_area, WaterfillSession& session){
    ShinySparkleSetSwSh sparkles;
    auto finder = session.make_iterator(20);
    WaterfillObject object;
    while (finder->find_next(object, true)){
        RadialSparkleDetector radial_sparkle(screen_area, object);
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
void ShinySparkleSetSwSh::read_from_image(size_t screen_area, const ImageViewRGB32& image){
    clear();
    if (!image){
        return;
    }

    std::vector<PackedBinaryMatrix> matrices = compress_rgb32_to_binary_range(
        image,
        {
            {0xffa0a000, 0xffffffff},
            {0xffb0b000, 0xffffffff},
            {0xffc0c000, 0xffffffff},
            {0xffd0d000, 0xffffffff},
        }
    );

    SpinLock lock;
    double best_alpha = 0;
    GlobalThreadPools::realtime_inference().run_in_parallel(
        [&](size_t index){
            auto session = make_WaterfillSession();
            session->set_source(matrices[index]);
            ShinySparkleSetSwSh sparkles = find_sparkles(screen_area, *session);
            sparkles.update_alphas();
            double alpha = sparkles.alpha_overall();

            WriteSpinLock lg(lock);
            if (best_alpha < alpha){
                best_alpha = alpha;
                *this = std::move(sparkles);
            }
        },
        0, matrices.size(), 1
    );

#if 0
    auto session = make_WaterfillSession();

    double best_alpha = 0;
    for (PackedBinaryMatrix& matrix : matrices){
        session->set_source(matrix);
        ShinySparkleSetSwSh sparkles = find_sparkles(screen_area, *session);
        sparkles.update_alphas();
        double alpha = sparkles.alpha_overall();
        if (best_alpha < alpha){
            best_alpha = alpha;
            *this = std::move(sparkles);
        }
    }
#endif
}




void ShinySparkleAggregator::add_frame(const std::shared_ptr<const ImageRGB32>& image, const ShinySparkleSetSwSh& sparkles){
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


