/*  Binary Image
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "BinaryImage.h"

namespace PokemonAutomation{



PackedBinaryMatrix2::PackedBinaryMatrix2(){
    m_matrix = Kernels::make_PackedBinaryMatrix(Kernels::get_BinaryMatrixType());
}
PackedBinaryMatrix2::PackedBinaryMatrix2(size_t width, size_t height){
    m_matrix = Kernels::make_PackedBinaryMatrix(Kernels::get_BinaryMatrixType(), width, height);
}





}
