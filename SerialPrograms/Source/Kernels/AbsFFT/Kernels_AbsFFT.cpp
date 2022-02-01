/*  ABS FFT
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels_AbsFFT_TwiddleTable.h"
#include "Kernels_AbsFFT_FullTransform.h"
#include "Kernels_AbsFFT.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{

TwiddleTable& global_table(){
    static TwiddleTable table(14);
    return table;
}


void fft_abs(int k, float* abs, float* real){
    if (k <= 0){
        throw "FFT length must be at least 2^1.";
    }
    if ((size_t)abs & 63){
        throw "abs must be aligned to 64 bytes.";
    }
    if ((size_t)real & 63){
        throw "real must be aligned to 64 bytes.";
    }

    TwiddleTable& table = global_table();
    table.ensure(k);
    fft_abs(table, k, abs, real);
}



}
}
}
