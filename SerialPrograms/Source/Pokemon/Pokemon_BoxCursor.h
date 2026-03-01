/*  Box Cursor
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_BoxCursor_H
#define PokemonAutomation_Pokemon_BoxCursor_H

#include <cstddef>
#include <ostream>

namespace PokemonAutomation{
namespace Pokemon{


// Constants for box dimensions
extern const size_t BOX_ROWS;
extern const size_t BOX_COLS;


// Represents a cursor position within Pokemon storage boxes
struct BoxCursor{
    // Convert a global index to a BoxCursor position
    BoxCursor(size_t index);
    BoxCursor(size_t box, size_t row, size_t column) : box(box), row(row), column(column) {}
    BoxCursor(){}

    size_t box = 0;
    size_t row = 0;
    size_t column = 0;
};

// Output operator for BoxCursor
std::ostream& operator<<(std::ostream& os, const BoxCursor& cursor);

// Convert a box cursor coordinate to a global index
size_t to_global_index(size_t box, size_t row, size_t column);


}
}
#endif
