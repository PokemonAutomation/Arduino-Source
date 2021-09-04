/*  Square Shiny Trigger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include <set>
#include "PokemonSwSh_SquareTrigger.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

//const double SQUARE_ANGLE_TOLERANCE_DEGREES = 30;



bool is_square_beam(
    const CellMatrix& matrix,
    const FillGeometry& object,
    double min_length
){
    pxint_t width = object.box.width();
    pxint_t height = object.box.height();
    if (width < 5 || height < 5){
        return false;
    }
    pxint_t min_pixel_width = (pxint_t)(matrix.width() * min_length);
    if (width < min_pixel_width){
        return false;
    }
    if ((size_t)width * width < object.area * 50){
        return false;
    }
    if (width < height * 10){
        return false;
    }

    //  Make sure it's actually a line.

    std::vector<pxint_t> heights(width);
    bool row_ok = false;
//    cout << "{";
    for (pxint_t r = object.box.min_y; r < object.box.max_y; r++){
//        cout << "    {";
        pxint_t length = 0;
        for (pxint_t c = object.box.min_x; c < object.box.max_x; c++){
//            cout << matrix[r][c] << ", ";
            pxint_t ok = matrix[r][c] == object.id ? 1 : 0;
            length += ok;
            heights[c - object.box.min_x] += ok;
        }
//        cout << "}," << endl;
        if (length >= min_pixel_width){
            row_ok = true;
        }
    }
//    cout << "}" << endl;
    if (!row_ok){
        return false;
    }

    //  Make sure the bulge is not on the edge.
    pxint_t thickest = 0;
    pxint_t thickest_column = 0;
    for (pxint_t c = 0; c < width; c++){
        pxint_t h = heights[c];
//        cout << h << ", ";
        if (thickest < h){
            thickest = h;
            thickest_column = c;
        }
    }
//    cout << "}" << endl;
//    cout << "thickest = " << thickest << endl;
    pxint_t diff_left = thickest_column;
    pxint_t diff_right = width - thickest_column;
//    cout << diff_left << " : " << diff_right << endl;
    if (diff_left * 10 < diff_right){
//        cout << "line is off-center" << endl;
        return false;
    }
    if (diff_right * 10 < diff_left){
//        cout << "line is off-center" << endl;
        return false;
    }

//    cout << width << " x " << height << endl;

    return true;
}




}
}
}

