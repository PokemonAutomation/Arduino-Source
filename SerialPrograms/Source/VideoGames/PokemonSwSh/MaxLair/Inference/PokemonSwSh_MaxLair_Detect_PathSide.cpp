/*  Max Lair Detect Path Side
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonTools/Images/DistanceToLine.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "PokemonSwSh_MaxLair_Detect_PathSide.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{

using namespace Kernels;
using namespace Kernels::Waterfill;


const double ARROW_MAX_DISTANCE = 0.04;

bool is_arrow_pointed_up(
    const PackedBinaryMatrix& matrix,
    PackedBinaryMatrix& inverted,
    const WaterfillObject& object
){
    size_t width = matrix.width();
    size_t height = matrix.height();

    std::unique_ptr<WaterfillSession> session = make_WaterfillSession(inverted);

    //  Verify left edge.
    WaterfillObject region0;
    {
        size_t topL_x = 0;
        for (; topL_x < width; topL_x++){
            if (matrix.get(topL_x, 0)){
                break;
            }
        }
        size_t left_y = 0;
        for (; left_y < height; left_y++){
            if (matrix.get(0, left_y)){
                break;
            }
        }

        DistanceToLine calc(
            topL_x, 0,
            0, left_y
        );
//        cout << "topL_x = " << topL_x << endl;
//        cout << "left_y = " << left_y << endl;

        if (!session->find_object_on_bit(region0, true, 0, 0)){
            return false;
        }
//        PackedBinaryMatrix matrix0 = region0.packed_matrix();
//        cout << region0.packed_matrix().dump() << endl;

        size_t border_count = 0;
        double border_sumsqr = 0;
        for (size_t r = region0.min_y; r < region0.max_y; r++){
            for (size_t c = region0.min_x; c < region0.max_x; c++){
                if (!region0.object->get(c, r)){
                    continue;
                }

                if ((c     > 0      && matrix.get(c - 1, r)) ||
                    (c + 1 < width  && matrix.get(c + 1, r)) ||
                    (r     > 0      && matrix.get(c, r - 1)) ||
                    (r + 1 < height && matrix.get(c, r + 1))
                ){
                    border_count++;
                    border_sumsqr += calc.distance_squared(c, r);
                }
            }
        }

        double distance = std::sqrt(border_sumsqr / border_count) / width;
//        cout << "left = " << distance << endl;
        if (distance > ARROW_MAX_DISTANCE){
            return false;
        }
    }

    //  Verify right edge.
    WaterfillObject region1;
    {
        size_t topR_x = width - 1;
        size_t right_y = 0;
        for (; right_y < height; right_y++){
            if (matrix.get(topR_x, right_y)){
                break;
            }
        }
        for (; topR_x > 0; topR_x--){
            if (matrix.get(topR_x, 0) == 1){
                break;
            }
        }

        DistanceToLine calc(
            topR_x, 0,
            width - 1, right_y
        );

//        cout << "width = " << width << endl;
//        cout << "topR_x = " << topR_x << endl;
//        cout << "right_y = " << right_y << endl;
//        cout << inverted.dump() << endl;
//        cout << inverted.dump_tiles() << endl;

        if (!session->find_object_on_bit(region1, true, width - 1, 0)){
            return false;
        }
//        cout << inverted.dump() << endl;
//        cout << inverted.dump_tiles() << endl;

//        cout << region1.object.dump() << endl;
//        cout << region1.packed_matrix().dump() << endl;
//        cout << region1.packed_matrix().dump_tiles() << endl;
//        cout << region1.width() << " x " << region1.height() << endl;

        size_t border_count = 0;
        double border_sumsqr = 0;
        for (size_t r = region1.min_y; r < region1.max_y; r++){
            for (size_t c = region1.min_x; c < region1.max_x; c++){
                if (!region1.object->get(c, r)){
                    continue;
                }
                if ((c     > 0      && matrix.get(c - 1, r)) ||
                    (c + 1 < width  && matrix.get(c + 1, r)) ||
                    (r     > 0      && matrix.get(c, r - 1)) ||
                    (r + 1 < height && matrix.get(c, r + 1))
                ){
                    border_count++;
                    border_sumsqr += calc.distance_squared(c, r);
                }
            }
        }

        double distance = std::sqrt(border_sumsqr / border_count) / width;
//        cout << "right = " << distance << endl;
        if (distance > ARROW_MAX_DISTANCE){
            return false;
        }
    }

    //  Verify bottom.
    WaterfillObject region2;
    if (!session->find_object_on_bit(region2, false, width - 1, height - 1)){
        return false;
    }

    if (region0.area + region1.area + region2.area < 0.9 * object.area){
        return false;
    }

//    cout << "good!" << endl;

    return true;
}
bool is_arrow_pointed_corner(
    const PackedBinaryMatrix& matrix,
    PackedBinaryMatrix& inverted
){
    size_t width = matrix.width();
    size_t height = matrix.height();

    std::unique_ptr<WaterfillSession> session = make_WaterfillSession(inverted);

    //  Verify right edge.
    {
        size_t right_y = 0;
        for (; right_y < height; right_y++){
            if (matrix.get(width - 1, right_y)){
                break;
            }
        }

        DistanceToLine calc(
            0, 0,
            width - 1, right_y
        );

        WaterfillObject region0;
        if (!session->find_object_on_bit(region0, true, width - 1, 0)){
            return false;
        }

        size_t border_count = 0;
        double border_sumsqr = 0;
        for (size_t r = region0.min_y; r < region0.max_y; r++){
            for (size_t c = region0.min_x; c < region0.max_x; c++){
                if (!region0.object->get(c, r)){
                    continue;
                }
                if ((c     > 0      && matrix.get(c - 1, r)) ||
                    (c + 1 < width  && matrix.get(c + 1, r)) ||
                    (r     > 0      && matrix.get(c, r - 1)) ||
                    (r + 1 < height && matrix.get(c, r + 1))
                ){
                    border_count++;
                    border_sumsqr += calc.distance_squared(c, r);
                }
            }
        }

        double distance = std::sqrt(border_sumsqr / border_count) / width;
//        cout << "right = " << distance << endl;
        if (distance > ARROW_MAX_DISTANCE){
            return false;
        }
    }

    //  Verify left edge.
    {
        size_t bottom_x = 0;
        for (; bottom_x < width; bottom_x++){
            if (matrix.get(bottom_x, height - 1)){
                break;
            }
        }

        DistanceToLine calc(
            0, 0,
            bottom_x, height - 1
        );

        WaterfillObject region1;
        if (!session->find_object_on_bit(region1, true, 0, height - 1)){
            return false;
        }

        size_t border_count = 0;
        double border_sumsqr = 0;
        for (size_t r = region1.min_y; r < region1.max_y; r++){
            for (size_t c = region1.min_x; c < region1.max_x; c++){
                if (!region1.object->get(c, r)){
                    continue;
                }
                if ((c     > 0      && matrix.get(c - 1, r)) ||
                    (c + 1 < width  && matrix.get(c + 1, r)) ||
                    (r     > 0      && matrix.get(c, r - 1)) ||
                    (r + 1 < height && matrix.get(c, r + 1))
                ){
                    border_count++;
                    border_sumsqr += calc.distance_squared(c, r);
                }
            }
        }

        double distance = std::sqrt(border_sumsqr / border_count) / width;
//        cout << "bottom = " << distance << endl;
        if (distance > ARROW_MAX_DISTANCE){
            return false;
        }
    }

    return true;
}


bool is_arrow(const ImageViewRGB32& image, const WaterfillObject& object){
    double area_ratio = object.area_ratio();
    if (area_ratio < 0.35 || area_ratio > 0.55){
        return false;
    }
    double aspect_ratio = object.aspect_ratio();
    if (aspect_ratio < 0.5 || aspect_ratio > 2.0){
        return false;
    }

    ImageRGB32 cropped = extract_box_reference(image, object).copy();

    PackedBinaryMatrix matrix = object.packed_matrix();
    filter_by_mask(matrix, cropped, Color(0), true);


    ImageStats stats = image_stats(cropped);
    if (!is_white(stats, 500, 100)){
        return false;
    }

    PackedBinaryMatrix inverted = matrix.copy();
    inverted.invert();

//    static int c = 0;
//    std::string file = "test-" + std::to_string(c++) + ".png";
//    cout << object.min_x << endl;
//    cropped0.save(file);

    if (is_arrow_pointed_up(matrix, inverted, object)){
//        cout << "up" << endl;
//        cropped0.save(file);
        return true;
    }

    if (is_arrow_pointed_corner(matrix, inverted)){
//        cout << "up" << endl;
//        cropped0.save(file);
        return true;
    }

    return false;
}



int8_t read_side(WaterfillSession& session, const ImageViewRGB32& image, uint8_t pixel_threshold){
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_min(
        image, pixel_threshold, pixel_threshold, pixel_threshold
    );

//    cout << "pixel_threshold = " << (int)pixel_threshold << endl;
    session.set_source(matrix);

    size_t min_area = image.total_pixels();
    min_area = (size_t)((double)300 * min_area / 1293312);

    auto finder = session.make_iterator(min_area);
    WaterfillObject arrow;
    WaterfillObject object;
    size_t count = 0;
    while (finder->find_next(object, true)){
        if (!is_arrow(image, object)){
            continue;
        }
        count++;
        arrow = object;
    }
//    cout << "count = " << count << endl;
    if (count != 1){
        return -1;
    }

//    cout << arrow.min_x << " | " << (size_t)image.width() / 3 << endl;

    return arrow.min_x < (size_t)image.width() / 3
        ? 0
        : 1;
}

int8_t read_side(const ImageViewRGB32& image){
//    cout << image.width() * image.height() << endl;
    auto session = make_WaterfillSession();
    int8_t ret;
    if ((ret = read_side(*session, image, 160)) != -1) return ret;
    if ((ret = read_side(*session, image, 176)) != -1) return ret;
    if ((ret = read_side(*session, image, 192)) != -1) return ret;
    if ((ret = read_side(*session, image, 208)) != -1) return ret;
    if ((ret = read_side(*session, image, 224)) != -1) return ret;
    return ret;
}



}
}
}
}
