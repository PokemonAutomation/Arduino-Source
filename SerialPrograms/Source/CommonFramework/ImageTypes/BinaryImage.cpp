/*  Binary Image
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "BinaryImage.h"

namespace PokemonAutomation{



PackedBinaryMatrix::PackedBinaryMatrix(){
    m_matrix = Kernels::make_PackedBinaryMatrix(Kernels::get_BinaryMatrixType());
}
PackedBinaryMatrix::PackedBinaryMatrix(size_t width, size_t height){
    m_matrix = Kernels::make_PackedBinaryMatrix(Kernels::get_BinaryMatrixType(), width, height);
}





}
