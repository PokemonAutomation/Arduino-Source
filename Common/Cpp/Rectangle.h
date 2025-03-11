/*  Rectangle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Rectangle_H
#define PokemonAutomation_Rectangle_H

namespace PokemonAutomation{


template <typename Type>
struct Rectangle{
    //  min is the start of the box.
    //  max is the end of the box.
    //  max - min is the width or height of the box.
    Type min_x;
    Type min_y;
    Type max_x;
    Type max_y;

    Rectangle() = default;
    Rectangle(Type p_min_x, Type p_min_y, Type p_max_x, Type p_max_y);

    Type width() const{ return max_x - min_x; }
    Type height() const{ return max_y - min_y; }
    Type area() const{ return width() * height(); }

    bool operator==(const Rectangle& box) const;

    //  Return whether two boxes overlap. Boxes touching each other does not count as overlap.
    bool overlaps_with(const Rectangle& box) const;
    Type overlapping_area(const Rectangle& box) const;

    //  Create a box covering both `this` box and the parameter `box` passed in.
    //  If the parameter `box` has 0 area, do no change.
    //  If `this` box has 0 area, `this` becomes the parameter `box`.
    void merge_with(const Rectangle& box);

    //  Whether a point (x, y) is inside the box.
    bool is_inside(Type x, Type y) const;           //  Excludes border.
    bool is_inside_or_on(Type x, Type y) const;     //  Includes border.

    //  Whether this box completely encloses "box".
    bool encloses(const Rectangle& box) const;      //  Includes border.

};


//
//  Implementations
//

template <typename Type>
bool Rectangle<Type>::operator==(const Rectangle& box) const{
    return
        min_x == box.min_x && min_y == box.min_y &&
        max_x == box.max_x && max_y == box.max_y;
}
template <typename Type>
bool Rectangle<Type>::overlaps_with(const Rectangle& box) const{
    return !(box.min_x >= max_x || box.max_x <= min_x || box.min_y >= max_y || box.max_y <= min_y);
}
template <typename Type>
bool Rectangle<Type>::is_inside(Type x, Type y) const{
    return x > min_x && x < max_x && y > min_y && y < max_y;
}
template <typename Type>
bool Rectangle<Type>::is_inside_or_on(Type x, Type y) const{
    return x >= min_x && x <= max_x && y >= min_y && y <= max_y;
}
template <typename Type>
bool Rectangle<Type>::encloses(const Rectangle& box) const{
    return min_x <= box.min_x && box.max_x <= max_x && min_y <= box.min_y && box.max_y <= max_y;
}




}
#endif
