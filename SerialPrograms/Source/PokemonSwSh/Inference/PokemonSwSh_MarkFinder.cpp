/*  Mark Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/FillGeometry.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonSwSh_MarkFinder.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



#if 0
struct WhiteFilter{
    size_t count = 0;

    void operator()(FillMatrix::ObjectID& cell, const QImage& image, pxint_t x, pxint_t y);
};
void WhiteFilter::operator()(FillMatrix::ObjectID& cell, const QImage& image, pxint_t x, pxint_t y){
    QRgb pixel = image.pixel(x, y);
    if ((pixel & 0x00808080) != 0x00808080){
        cell = 0;
        return;
    }

    unsigned r = qRed(pixel);
    unsigned g = qGreen(pixel);
    unsigned b = qBlue(pixel);
    unsigned average = (r + g + b) / 3;
    r -= average;
    g -= average;
    b -= average;
    r *= r;
    g *= g;
    b *= b;
    unsigned variation = r + g + b;

    int set = variation < 2000 ? 1 : 0;
    cell = set;
    count += set;
}
struct WhiteFilter2{
    size_t count = 0;

    void operator()(FillMatrix::ObjectID& cell, QImage& image, pxint_t x, pxint_t y);
};
void WhiteFilter2::operator()(FillMatrix::ObjectID& cell, QImage& image, pxint_t x, pxint_t y){
    QRgb pixel = image.pixel(x, y);
    if ((pixel & 0x00808080) != 0x00808080){
        cell = 0;
        image.setPixel(x, y, 0);
        return;
    }

    unsigned r = qRed(pixel);
    unsigned g = qGreen(pixel);
    unsigned b = qBlue(pixel);
    unsigned average = (r + g + b) / 3;
    r -= average;
    g -= average;
    b -= average;
    r *= r;
    g *= g;
    b *= b;
    unsigned variation = r + g + b;

    int set = variation < 1000 ? 1 : 0;
    if (!set){
        image.setPixel(x, y, 0);
    }
    cell = set;
    count += set;
}
#endif



struct MarkFilter{
    size_t count = 0;

    void operator()(CellMatrix::ObjectID& cell, const QImage& image, pxint_t x, pxint_t y){
        QRgb pixel = image.pixel(x, y);
        if ((pixel & 0x00808080) != 0x00808080){
            cell = 0;
        }else{
            cell = 1;
            count++;
        }
    }
};
struct MarkFilterDebug{
    size_t count = 0;

    void operator()(CellMatrix::ObjectID& cell, QImage& image, pxint_t x, pxint_t y){
        QRgb pixel = image.pixel(x, y);
        if ((pixel & 0x00808080) != 0x00808080){
            cell = 0;
            image.setPixel(x, y, 0);
        }else{
            cell = 1;
            count++;
        }
    }
};



bool is_exclamation_mark(
    const FillGeometry& top,
    const FillGeometry& bot
){
//    cout << "asdf" << endl;

    //  Make sure dimensions are reasonable.
//    if (!top.assert_aspect_ratio(0.3, 0.7)){
//        return false;
//    }
    if (!bot.assert_aspect_ratio(0.5, 2.0)){
        return false;
    }

//    cout << "=========" << endl;
//    cout << top.box.width() << " x " << top.box.height() << endl;
//    cout << bot.box.width() << " x " << bot.box.height() << endl;
//    cout << "check 1" << endl;

//    if (top.area < 100 || !top.assert_area_ratio(0.65, 0.75)){
//        return false;
//    }
    if (!bot.assert_area_ratio(0.80, 1.0)){
//        cout << "Bad bot area." << endl;
        return false;
    }

//    if (top.area > 8 * bot.area){
//        cout << "Ratio too large: " << top.area << " / " << bot.area << endl;
//        return false;
//    }
    if (top.area < 4 * bot.area){
//        cout << "Ratio too small" << endl;
        return false;
    }

    //  Verify that top is above bottom.
    if (top.box.max_y >= bot.box.min_y){
        return false;
    }
//    cout << "check 1" << endl;

    //  Verify bottom is left of the top.
    if (top.center_x() < bot.center_x()){
        return false;
    }

    //  Make sure horizontal alignment is reasonable.
    pxint_t mid = (top.box.min_x + top.box.max_x) >> 1;
    pxint_t diff = bot.box.max_x > mid
        ? bot.box.max_x - mid
        : mid - bot.box.max_x;
    if (diff * 3 > top.box.width()){
//        cout << "Bad horizontal alignment." << endl;
        return false;
    }

    //  Make sure vertical alignment is reasonable.
    if (top.box.max_y + top.box.height() <= bot.box.min_y){
        return false;
    }

#if 0
    cout << "area = " << top.area << " / " << bot.area << endl;
    cout << "top = " << top.box.width() << " x " << top.box.height() << ", " << top.area_ratio() << endl;
    cout << "bot = " << bot.box.width() << " x " << bot.box.height() << ", " << bot.area_ratio() << endl;
#endif

    return true;
}

bool is_question_mark(
    const FillGeometry& top,
    const FillGeometry& bot
){
//    //  Make sure dimensions are reasonable.
//    double top_aspect_ratio = (double)top.box.width() / top.box.height();
//    if (0.8 > top_aspect_ratio || top_aspect_ratio > 1.2){
//        return false;
//    }

    if (!bot.assert_aspect_ratio(0.5, 2.0)){
        return false;
    }


    if (bot.area * 2 < (size_t)bot.box.width() * bot.box.height()){
        return false;
    }

    //  Verify that the top part of the question is between 5-10x the area of the bottom.
//    if (top.area > 10 * bot.area){
//        cout << "bad 0" << endl;
//        return false;
//    }
    if (top.area < 5 * bot.area){
//        cout << "bad 1" << endl;
        return false;
    }

    //  Verify that top is completely above the bottom.
    if (top.box.max_y >= bot.box.min_y){
        return false;
    }

    //  Verify that bottom isn't too far below the top.
    if (top.box.max_y + bot.box.height() < bot.box.min_y){
        return false;
    }

    //  Verify that bottom is completely within the x-axis of the top.
    if (bot.box.min_x < top.box.min_x){
        return false;
    }
    if (bot.box.max_x > top.box.max_x){
        return false;
    }

//    cout << top.box.width() << " x " << top.box.height() << " : " << top.area << " / " << top.box.width() * top.box.height() << endl;

#if 0
    cout << "area = " << top.area << " / " << bot.area << endl;
    cout << "top = " << top.box.width() << " x " << top.box.height() << ", " << top.area_ratio() << endl;
    cout << "bot = " << bot.box.width() << " x " << bot.box.height() << ", " << bot.area_ratio() << endl;
#endif

    return true;
}

size_t find_exclamation_marks(
    QImage image, const CellMatrix& matrix,
    const FillGeometry& object,
    std::vector<ImagePixelBox>& marks
){
    CellMatrix search_matrix(image);
    for (pxint_t r = 0; r < image.height(); r++){
        for (pxint_t c = 0; c < image.width(); c++){
            if (matrix[r][c] != 1){
                search_matrix[r][c] = 0;
                continue;
            }
            QRgb pixel = image.pixel(c, r);
            int red = qRed(pixel);
            int green = qGreen(pixel);
//            cout << red << ", " << green << endl;
            CellMatrix::ObjectID cell = red >= 96 && green <= 160 ? 1 : 0;
            if (cell == 0){
                image.setPixel(c, r, 0);
            }
            search_matrix[r][c] = cell;
        }
    }

#if 0
    static int c = 0;
    cout << "running: " << c << endl;
    image.save("exclamation-filter-" + QString::number(c++) + ".png");
    if (c != 5){
        return 0;
    }
    if (matrix.width() * matrix.height() < 10000){
        cout << search_matrix.dump() << endl;
    }
#endif


    std::vector<FillGeometry> objects = find_all_objects(search_matrix, 1, false, 10);
//    cout << "objects = " << objects.size() << endl;
    if (objects.size() > 100){
//        cout << "objects.size() = " << objects.size() << endl;
//        static int c = 0;
//        image.save("test-" + QString::number(c++) + ".png");
        return 0;
    }

    size_t count = 0;
    for (const FillGeometry& top : objects){
        //  Throw out obvious bad top candidates.
        if (top.area < 50 || !top.assert_area_ratio(0.55, 0.80)){
//            cout << "Bad top area: area = " << top.area << ", ratio = " << top.area_ratio() << endl;
            continue;
        }
        if (!top.assert_aspect_ratio(0.3, 0.7)){
//            cout << "Bad top aspect ratio: " << top.aspect_ratio() << endl;
            continue;
        }

//        cout << top.box.width() << " x " << top.box.height() << endl;
        for (const FillGeometry& bot : objects){
            if (!is_exclamation_mark(top, bot)){
                continue;
            }
            ImagePixelBox box{
                top.box.min_x + object.box.min_x,
                top.box.min_y + object.box.min_y,
                top.box.max_x + object.box.min_x,
                bot.box.max_y + object.box.min_y
            };

//            FillMatrix submatrix = matrix.extract(PixelBox{top.box.min_x, top.box.min_y, top.box.max_x, bot.box.max_y});
//            cout << submatrix.dump() << endl;

            marks.emplace_back(box);
            count++;
        }
    }

#if 0
    if (count != 0){
        static int c = 0;
        const PixelBox& box = marks.back();
        image.copy(
            box.min_x,
            box.min_y,
            box.width(), box.height()
        ).save("detection-" + QString::number(c++) + ".png");
        cout << "found" << endl;
    }
#endif

    return count;
}
size_t find_question_marks(
    const QImage& image, const CellMatrix& matrix,
    const FillGeometry& object,
    std::vector<ImagePixelBox>& marks
){
    CellMatrix search_matrix(image);
    for (int r = 0; r < image.height(); r++){
        for (int c = 0; c < image.width(); c++){
            if (matrix[r][c] != 1){
                search_matrix[r][c] = 0;
                continue;
            }
            QRgb pixel = image.pixel(c, r);
            int blue = qBlue(pixel);
            search_matrix[r][c] = blue >= 128 ? 1 : 0;
        }
    }

#if 0
    static int c = 0;
    cout << "running: " << c << endl;
    image.save("question-filter-" + QString::number(c++) + ".png");
    if (c != 3){
        return 0;
    }
    if (matrix.width() * matrix.height() < 10000){
        cout << search_matrix.dump() << endl;
    }
#endif

    std::vector<FillGeometry> objects = find_all_objects(search_matrix, 1, false, 4);
    if (objects.size() > 5){
        return 0;
    }

    size_t count = 0;
    for (const FillGeometry& top : objects){
//        cout << top.box.width() << " x " << top.box.height() << endl;
        if (top.area < 50 || !top.assert_area_ratio(0.40, 0.55)){
//            cout << "Bad top area: area = " << top.area << ", ratio = " << top.area_ratio() << endl;
            continue;
        }
        if (!top.assert_aspect_ratio(0.65, 1.0)){
//            cout << "Bad top aspect ratio: " << top.aspect_ratio() << endl;
            continue;
        }

        for (const FillGeometry& bot : objects){
            if (!is_question_mark(top, bot)){
                continue;
            }
            ImagePixelBox box{
                top.box.min_x + object.box.min_x,
                top.box.min_y + object.box.min_y,
                top.box.max_x + object.box.min_x,
                bot.box.max_y + object.box.min_y
            };
            marks.emplace_back(box);
            count++;
        }
    }

    return count;
}

size_t find_marks(
    const QImage& image, const CellMatrix& matrix,
    const FillGeometry& object,
    std::vector<ImagePixelBox>* exclamation_marks,
    std::vector<ImagePixelBox>* question_marks
){
    pxint_t width = object.box.width();
    pxint_t height = object.box.height();
//    if (object.area > 10000){
//        cout << "too big: " << object.area << endl;
//        return 0;
//    }

//    cout << "area = " << object.area << endl;

    CellMatrix edge_matrix = matrix.extract(object.box, object.id);
    //  Invert the cells.
    for (pxint_t r = 0; r < height; r++){
        for (pxint_t c = 0; c < width; c++){
            CellMatrix::ObjectID cell = edge_matrix[r][c];
            cell = cell ? 0 : 1;
            edge_matrix[r][c] = cell;
        }
    }
    {
        //  Fill in the edges.
        size_t edge_area = 0;
        for (pxint_t c = 0; c < width; c++){
            FillGeometry body;
            if (fill_geometry(body, edge_matrix, 1, c, 0, false, 2)){
                edge_area += body.area;
            }
            if (fill_geometry(body, edge_matrix, 1, c, height - 1, false, 2)){
                edge_area += body.area;
            }
        }
        for (pxint_t r = 0; r < height; r++){
            FillGeometry body;
            if (fill_geometry(body, edge_matrix, 1, 0, r, false, 2)){
                edge_area += body.area;
            }
            if (fill_geometry(body, edge_matrix, 1, width - 1, r, false, 2)){
                edge_area += body.area;
            }
        }
    }

//    QImage copy = image;
    QImage subimage = extract_box(image, object.box);

    size_t count = 0;
    if (exclamation_marks){
        count += find_exclamation_marks(subimage, edge_matrix, object, *exclamation_marks);
    }
    if (question_marks){
        count += find_question_marks(subimage, edge_matrix, object, *question_marks);
    }

//    static size_t id = 0;
//    if (count != 0){
//        subimage.save("detection-" + QString::number(id++) + ".png");
//    }
    return count;
}


size_t find_marks(
    const QImage& image,
    std::vector<ImagePixelBox>* exclamation_marks,
    std::vector<ImagePixelBox>* question_marks
){
    CellMatrix matrix(image);
    MarkFilter filter;
    matrix.apply_filter(image, filter);

    size_t count = 0;
    std::vector<FillGeometry> white_boxes = find_all_objects(matrix, 1, true, 100);
//    cout << "white_boxes = " << white_boxes.size() << endl;
    for (const FillGeometry& white_box : white_boxes){
        count += find_marks(image, matrix, white_box, exclamation_marks, question_marks);
    }

#if 0
    if (count > 0 || true){
        static int c = 0;
        image.save("white_filter-" + QString::number(c) + "-O.png");
        copy.save("white_filter-" + QString::number(c) + "-P.png");
        c++;
    }
#endif

    return count;
}



}
}
}
