/*  Max Lair Detect Path Side
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/ImageTools/CommonFilters.h"
#include "CommonFramework/ImageTools/DistanceToLine.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonSwSh_MaxLair_Detect_PathSide.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


const double ARROW_MAX_DISTANCE = 0.04;

bool is_arrow_pointed_up(const CellMatrix& matrix, const FillGeometry& object){
    CellMatrix submatrix = matrix.extract(object.box, object.id);
    pxint_t width = submatrix.width();
    pxint_t height = submatrix.height();

//    cout << submatrix.dump() << endl;


    //  Verify left edge.
    FillGeometry region0;
    {
        pxint_t topL_x = 0;
        for (; topL_x < width; topL_x++){
            if (submatrix[0][topL_x] == 1){
                break;
            }
        }
        pxint_t left_y = 0;
        for (; left_y < height; left_y++){
            if (submatrix[left_y][0] == 1){
                break;
            }
        }

        DistanceToLine calc(
            topL_x, 0,
            0, left_y
        );
//        cout << "topL_x = " << topL_x << endl;
//        cout << "left_y = " << left_y << endl;

        if (!fill_geometry(region0, submatrix, 0, 0, 0, false, 2)){
            return false;
        }

        size_t border_count = 0;
        double border_sumsqr = 0;
        for (pxint_t r = 0; r < height; r++){
            for (pxint_t c = 0; c < width; c++){
                if (submatrix[r][c] != 2){
                    continue;
                }
                if ((c     > 0      && submatrix[r][c - 1] == 1) ||
                    (c + 1 < width  && submatrix[r][c + 1] == 1) ||
                    (r     > 0      && submatrix[r - 1][c] == 1) ||
                    (r + 1 < height && submatrix[r + 1][c] == 1)
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
    FillGeometry region1;
    {
        pxint_t topR_x = width - 1;
        pxint_t right_y = 0;
        for (; right_y < height; right_y++){
            if (submatrix[right_y][topR_x] == 1){
                break;
            }
        }
        for (; topR_x >= 0; topR_x--){
            if (submatrix[0][topR_x] == 1){
                break;
            }
        }

        DistanceToLine calc(
            topR_x, 0,
            width - 1, right_y
        );

//        cout << "topR_x = " << topR_x << endl;
//        cout << "right_y = " << right_y << endl;

        if (!fill_geometry(region1, submatrix, 0, submatrix.width() - 1, 0, false, 3)){
            return false;
        }

        size_t border_count = 0;
        double border_sumsqr = 0;
        for (pxint_t r = 0; r < height; r++){
            for (pxint_t c = 0; c < width; c++){
                if (submatrix[r][c] != 3){
                    continue;
                }
                if ((c     > 0      && submatrix[r][c - 1] == 1) ||
                    (c + 1 < width  && submatrix[r][c + 1] == 1) ||
                    (r     > 0      && submatrix[r - 1][c] == 1) ||
                    (r + 1 < height && submatrix[r + 1][c] == 1)
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

    FillGeometry region2;
    if (!fill_geometry(region2, submatrix, 0, submatrix.width() - 1, submatrix.height() - 1, false, 4)){
        return false;
    }


    if (region0.area + region1.area + region2.area < 0.9 * object.area){
        return false;
    }


//    cout << "asdf" << endl;
//    cout << submatrix.dump() << endl;

    return true;
}
bool is_arrow_pointed_corner(const CellMatrix& matrix, const FillGeometry& object){
    CellMatrix submatrix = matrix.extract(object.box, object.id);
    pxint_t width = submatrix.width();
    pxint_t height = submatrix.height();


    //  Verify right edge.
    {
        pxint_t right_y = 0;
        for (; right_y < height; right_y++){
            if (submatrix[right_y][width - 1] == 1){
                break;
            }
        }

        DistanceToLine calc(
            0, 0,
            width - 1, right_y
        );

        FillGeometry triangle;
        if (!fill_geometry(triangle, submatrix, 0, width - 1, 0, false, 2)){
            return false;
        }

        size_t border_count = 0;
        double border_sumsqr = 0;
        for (pxint_t r = 0; r < height; r++){
            for (pxint_t c = 0; c < width; c++){
                if (submatrix[r][c] != 2){
                    continue;
                }
                if ((c     > 0      && submatrix[r][c - 1] == 1) ||
                    (c + 1 < width  && submatrix[r][c + 1] == 1) ||
                    (r     > 0      && submatrix[r - 1][c] == 1) ||
                    (r + 1 < height && submatrix[r + 1][c] == 1)
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
        pxint_t bottom_x = 0;
        for (; bottom_x < width; bottom_x++){
            if (submatrix[height - 1][bottom_x] == 1){
                break;
            }
        }

        DistanceToLine calc(
            0, 0,
            bottom_x, height - 1
        );

        FillGeometry triangle;
        if (!fill_geometry(triangle, submatrix, 0, 0, height - 1, false, 3)){
            return false;
        }

        size_t border_count = 0;
        double border_sumsqr = 0;
        for (pxint_t r = 0; r < height; r++){
            for (pxint_t c = 0; c < width; c++){
                if (submatrix[r][c] != 3){
                    continue;
                }
                if ((c     > 0      && submatrix[r][c - 1] == 1) ||
                    (c + 1 < width  && submatrix[r][c + 1] == 1) ||
                    (r     > 0      && submatrix[r - 1][c] == 1) ||
                    (r + 1 < height && submatrix[r + 1][c] == 1)
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

bool is_arrow(const QImage& image, const CellMatrix& matrix, const FillGeometry& object){
    double area_ratio = (double)object.area / object.box.area();
    if (area_ratio < 0.35 || area_ratio > 0.55){
        return false;
    }
    ImageStats stats = object_stats(image, matrix, object);
//    cout << stats.average << stats.stddev << endl;
    if (!is_white(stats, 500, 100)){
        return false;
    }

//    CellMatrix submatrix = matrix.extract(object.box, object.id);
//    cout << submatrix.dump() << endl;

    if (is_arrow_pointed_up(matrix, object)){
//        cout << "up" << endl;
        return true;
    }
    if (is_arrow_pointed_corner(matrix, object)){
//        cout << "corner" << endl;
        return true;
    }

    return false;
}



int8_t read_side(const QImage& image, int p_min_rgb_sum){
    CellMatrix matrix(image);
    BrightFilter filter(p_min_rgb_sum);
    matrix.apply_filter(image, filter);

    std::vector<FillGeometry> objects = find_all_objects(matrix, 1, false, 300);

//    std::deque<InferenceBoxScope> hits;
//    cout << p_min_rgb_sum << endl;

    FillGeometry arrow;
    size_t count = 0;
    for (const FillGeometry& item : objects){

//        image.copy(
//            item.box.min_x, item.box.min_y, item.box.width(), item.box.height()
//        ).save("test-" + QString::number(p_min_rgb_sum) + "-" + QString::number(count) + ".png");

        if (is_arrow(image, matrix, item)){
//            cout << item.area << " / " << item.box.area() << endl;
//            image.copy(
//                item.box.min_x, item.box.min_y, item.box.width(), item.box.height()
//            ).save("test-" + QString::number(p_min_rgb_sum) + "-" + QString::number(count) + ".png");

            arrow = item;
            count++;
        }
//        hits.emplace_back(overlay, translate_to_parent(screen, box, item.box), Qt::green);
    }
//    cout << "count = " << count << endl;
    if (count != 1){
        return -1;
    }

    return arrow.box.min_x < image.width() / 3
        ? 0
        : 1;
}

int8_t read_side(const QImage& image){
    int8_t ret;
    if ((ret = read_side(image, 500)) != -1) return ret;
    if ((ret = read_side(image, 550)) != -1) return ret;
    if ((ret = read_side(image, 600)) != -1) return ret;
    if ((ret = read_side(image, 650)) != -1) return ret;
    if ((ret = read_side(image, 700)) != -1) return ret;
    return ret;
}


}
}
}
}
