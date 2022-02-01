/*  ABS FFT Twiddle Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_TwiddleTable_H
#define PokemonAutomation_Kernels_AbsFFT_TwiddleTable_H

#include <atomic>
#include "Common/Cpp/AlignedVector.h"
#include "Common/Cpp/SpinLock.h"
#include "Kernels_AbsFFT_Arch.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{


AlignedVector<vcomplex> make_table_row(int k, double angle_multiplier, size_t length_multipler);



struct PerSizeTables{
    AlignedVector<vcomplex> w1;
    AlignedVector<vcomplex> w3;
};


class TwiddleTable{
public:
    ~TwiddleTable();
    TwiddleTable(int initial_size = 14);

    const PerSizeTables& operator[](int k) const;
    void ensure(int k);

private:
    void expand(int k);

private:
    std::atomic<int> m_size_k;
    SpinLock m_resize_lock;
    PerSizeTables m_tables[32];
};



struct TableRowReader{
    using vtype = Intrinsics::vtype;

    const vcomplex* data;

    TableRowReader(const vcomplex* p_data)
        : data(p_data)
    {}


    PA_FORCE_INLINE void get(float& real, float& imag, size_t index) const{
        const vcomplex& point = data[index / VECTOR_LENGTH];
        size_t sindex = index % VECTOR_LENGTH;
        real = point.real(sindex);
        imag = point.imag(sindex);
    }
#if 0
    PA_FORCE_INLINE void get(vtype& real, vtype& imag, size_t index) const{
        const vcomplex& point = data[index];
        real = point.r;
        imag = point.i;
    }
#endif

};



}
}
}
#endif
