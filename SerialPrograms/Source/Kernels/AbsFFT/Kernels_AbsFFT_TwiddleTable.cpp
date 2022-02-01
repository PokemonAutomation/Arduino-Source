/*  ABS FFT Twiddle Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "Common/Cpp/AlignedVector.tpp"
#include "Kernels_AbsFFT_TwiddleTable.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{


AlignedVector<vcomplex> make_table_row(int k, double angle_multiplier, size_t length_multipler){
    if (k < 2){
        return AlignedVector<vcomplex>();
    }
    size_t length = (size_t)1 << (k - 2);
    size_t vlen = length * length_multipler / VECTOR_LENGTH;
    if (vlen < Intrinsics::MIN_TABLE_WIDTH){
        vlen = Intrinsics::MIN_TABLE_WIDTH;
    }
    AlignedVector<vcomplex> row(vlen);

    double w = 1.5707963267948966192 * angle_multiplier / length;

    for (size_t v = 0; v < vlen; v++){
        vcomplex& vec = row[v];
        for (size_t i = 0; i < VECTOR_LENGTH; i++){
            double angle = (double)(v * VECTOR_LENGTH + i) * w;
            vec.real(i) = (float)std::cos(angle);
            vec.imag(i) = (float)std::sin(angle);
        }
    }

    return row;
}



TwiddleTable::~TwiddleTable(){}
TwiddleTable::TwiddleTable(int initial_size)
    : m_size_k(0)
{
    expand(initial_size);
}

const PerSizeTables& TwiddleTable::operator[](int k) const{
    int size_k = m_size_k.load(std::memory_order_acquire);
    if (k > size_k){
        throw "Twiddle table is too small.";
    }
    return m_tables[k];
}

void TwiddleTable::ensure(int k){
    int size_k = m_size_k.load(std::memory_order_acquire);
    if (size_k >= k){
        return;
    }
    expand(k);
}
void TwiddleTable::expand(int k){
    SpinLockGuard lg(m_resize_lock);
    int size_k = m_size_k.load(std::memory_order_acquire);
    if (size_k >= k){
        return;
    }
    for (; size_k < k;){
        size_k++;
        m_tables[size_k].w1 = make_table_row(size_k, 1, 2);
        m_tables[size_k].w3 = make_table_row(size_k, 3, 1);
        m_size_k.store(size_k, std::memory_order_release);
    }
}




}
}
}
