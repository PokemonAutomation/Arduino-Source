/*  Mark Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/FillGeometry.h"
#include "PokemonSwSh_MarkFinder.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



void WhiteFilter::operator()(FillMatrix::ObjectID& cell, const QImage& image, size_t x, size_t y){
    QRgb pixel = image.pixel(x, y);
    int set = (pixel & 0x00808080) == 0x00808080 ? 1 : 0;
//        if (!set){
//            image.setPixel(x, y, 0);
//        }
    cell = set;
    count += set;
}


struct WhiteFilter2{
    size_t count = 0;

    void operator()(FillMatrix::ObjectID& cell, QImage& image, size_t x, size_t y){
        QRgb pixel = image.pixel(x, y);
        int set = (pixel & 0x00808080) == 0x00808080 ? 1 : 0;
        if (!set){
            image.setPixel(x, y, 0);
        }
        cell = set;
        count += set;
    }
};




bool is_exclamation_mark(
    const FillGeometry& top,
    const FillGeometry& bot
){

    //  Make sure dimensions are reasonable.
    double top_aspect_ratio = (double)top.box.width() / top.box.height();
    if (0.4 > top_aspect_ratio || top_aspect_ratio > 0.6){
        return false;
    }
    double bot_aspect_ratio = (double)bot.box.width() / bot.box.height();
    if (0.5 > bot_aspect_ratio || bot_aspect_ratio > 2.0){
        return false;
    }

//    cout << "=========" << endl;
//    cout << top.box.width() << " x " << top.box.height() << endl;
//    cout << bot.box.width() << " x " << bot.box.height() << endl;
//    cout << "check 1" << endl;


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
    if (top.center_x < bot.center_x){
        return false;
    }

    //  Make sure horizontal alignment is reasonable.
    int mid = (top.box.min_x + top.box.max_x) / 2;
    if (std::abs(bot.box.max_x - mid) * 3 > top.box.width()){
//        cout << "Bad horizontal alignment." << endl;
        return false;
    }

    //  Make sure vertical alignment is reasonable.
    if (top.box.max_y + top.box.height() <= bot.box.min_y){
        return false;
    }

    return true;
}

bool is_question_mark(
    const FillGeometry& top,
    const FillGeometry& bot
){
    //  Make sure dimensions are reasonable.
    double top_aspect_ratio = (double)top.box.width() / top.box.height();
    if (0.8 > top_aspect_ratio || top_aspect_ratio > 1.2){
        return false;
    }


    if (bot.area * 2 < (size_t)bot.box.width() * bot.box.height()){
        return false;
    }

    //  Verify that the top part of the question is between 5-10x the area of the bottom.
    if (top.area > 10 * bot.area){
        return false;
    }
    if (top.area < 5 * bot.area){
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

    return true;
}

size_t find_exclamation_marks(
    const QImage& image, const FillMatrix& matrix,
    const FillGeometry& object,
    std::vector<PixelBox>& marks
){
    FillMatrix search_matrix(image);
    for (int r = 0; r < image.height(); r++){
        for (int c = 0; c < image.width(); c++){
            if (matrix[r][c] != 1){
                search_matrix[r][c] = 0;
                continue;
            }
            QRgb pixel = image.pixel(c, r);
            int red = qRed(pixel);
            int green = qGreen(pixel);
            search_matrix[r][c] = red >= 96 && green <= 160 ? 1 : 0;
        }
    }

    std::vector<FillGeometry> objects = find_all_objects(search_matrix, 1, false, 4);
    if (objects.size() > 5){
        return 0;
    }

    size_t count = 0;
    for (const FillGeometry& top : objects){
//        cout << top.box.width() << " x " << top.box.height() << endl;
        for (const FillGeometry& bot : objects){
            if (!is_exclamation_mark(top, bot)){
                continue;
            }
            PixelBox box{
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
size_t find_question_marks(
    const QImage& image, const FillMatrix& matrix,
    const FillGeometry& object,
    std::vector<PixelBox>& marks
){
    FillMatrix search_matrix(image);
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

    std::vector<FillGeometry> objects = find_all_objects(search_matrix, 1, false, 4);
    if (objects.size() > 5){
        return 0;
    }

    size_t count = 0;
    for (const FillGeometry& top : objects){
//        cout << top.box.width() << " x " << top.box.height() << endl;
        for (const FillGeometry& bot : objects){
            if (!is_question_mark(top, bot)){
                continue;
            }
            PixelBox box{
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
    const QImage& image, const FillMatrix& matrix,
    const FillGeometry& object,
    std::vector<PixelBox>* exclamation_marks,
    std::vector<PixelBox>* question_marks
){
    int width = object.box.width();
    int height = object.box.height();
//    if (object.area > 10000){
//        cout << "too big: " << object.area << endl;
//        return 0;
//    }

    //  Find the holes.

    FillMatrix edge_matrix = matrix.extract(object.box, object.id);
    //  Invert the cells.
    for (int r = 0; r < height; r++){
        for (int c = 0; c < width; c++){
            FillMatrix::ObjectID cell = edge_matrix[r][c];
            cell = cell ? 0 : 1;
            edge_matrix[r][c] = cell;
        }
    }
    {
        //  Fill in the edges.
        size_t edge_area = 0;
        for (int c = 0; c < width; c++){
            FillGeometry body;
            if (fill_geometry(body, edge_matrix, 1, c, 0, false, 2)){
                edge_area += body.area;
            }
            if (fill_geometry(body, edge_matrix, 1, c, height - 1, false, 2)){
                edge_area += body.area;
            }
        }
        for (int r = 0; r < height; r++){
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

    static size_t id = 0;
    if (count != 0){
//        subimage.save("mark-" + QString::number(id++) + ".png");
    }
    return count;
}


size_t find_marks(
    const QImage& image,
    std::vector<PixelBox>* exclamation_marks,
    std::vector<PixelBox>* question_marks
){
#if 1
    FillMatrix matrix(image);
    WhiteFilter filter;
    matrix.apply_filter(image, filter);
#else
    QImage copy = image;

    FillMatrix matrix(image);
    WhiteFilter2 filter;
    matrix.apply_filter(copy, filter);
    copy.save("white_filter.png");
#endif

    size_t count = 0;
    std::vector<FillGeometry> white_boxes = find_all_objects(matrix, true, 100);
    for (const FillGeometry& white_box : white_boxes){
        count += find_marks(image, matrix, white_box, exclamation_marks, question_marks);
    }

    return count;
}



}
}
}
