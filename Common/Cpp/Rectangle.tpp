/*  Rectangle
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Rectangle_TPP
#define PokemonAutomation_Rectangle_TPP

#include <algorithm>
#include "Common/Cpp/Exceptions.h"
#include "Rectangle.h"

namespace PokemonAutomation{


template <typename Type>
Rectangle<Type>::Rectangle(Type p_min_x, Type p_min_y, Type p_max_x, Type p_max_y)
    : min_x(p_min_x) , min_y(p_min_y) , max_x(p_max_x) , max_y(p_max_y)
{
    if (min_x > max_x){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Invalid Box: min_x = " + std::to_string(min_x) + ", max_x = " + std::to_string(max_x)
        );
    }
    if (min_y > max_y){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Invalid Box: min_y = " + std::to_string(min_y) + ", max_y = " + std::to_string(max_y)
        );
    }
}
template <typename Type>
void Rectangle<Type>::merge_with(const Rectangle& box){
    if (box.area() == 0){
        return;
    }
    if (this->area() == 0){
        *this = box;
    }
    min_x = std::min(min_x, box.min_x);
    min_y = std::min(min_y, box.min_y);
    max_x = std::max(max_x, box.max_x);
    max_y = std::max(max_y, box.max_y);
}
template <typename Type>
Type Rectangle<Type>::overlapping_area(const Rectangle& box) const{
    Type min_x = std::max(this->min_x, box.min_x);
    Type max_x = std::min(this->max_x, box.max_x);
    if (min_x >= max_x){
        return 0;
    }
    Type min_y = std::max(this->min_y, box.min_y);
    Type max_y = std::min(this->max_y, box.max_y);
    if (min_y >= max_y){
        return 0;
    }
    return (max_x - min_x) * (max_y - min_y);
}



}
#endif
