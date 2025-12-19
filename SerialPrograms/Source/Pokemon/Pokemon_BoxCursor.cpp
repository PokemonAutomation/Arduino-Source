/*  Box Cursor
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Pokemon_BoxCursor.h"

namespace PokemonAutomation{
namespace Pokemon{


const size_t BOX_ROWS = 5;
const size_t BOX_COLS = 6;


std::ostream& operator<<(std::ostream& os, const BoxCursor& cursor){
    os << "(" << cursor.box << "/" << cursor.row << "/" << cursor.column << ")";
    return os;
}

BoxCursor::BoxCursor(size_t index){
    column = index % BOX_COLS;
    index = index / BOX_COLS;
    row = index % BOX_ROWS;
    box = index / BOX_ROWS;
}

size_t to_global_index(size_t box, size_t row, size_t column){
    return box * BOX_ROWS * BOX_COLS + row * BOX_COLS + column;
}


}
}
