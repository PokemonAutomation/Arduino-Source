/*  ABS FFT Twiddle Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_TwiddleTable_H
#define PokemonAutomation_Kernels_AbsFFT_TwiddleTable_H

#include <atomic>
#include "Common/Cpp/Containers/AlignedVector.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Kernels_AbsFFT_Arch.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{



template <typename Context>
AlignedVector<vcomplex<Context>> make_table_row(int k, double angle_multiplier, size_t length_multipler);


template <typename Context>
struct PerSizeTables{
    AlignedVector<vcomplex<Context>> w1;
    AlignedVector<vcomplex<Context>> w3;
};


template <typename Context>
class TwiddleTable{
public:
    ~TwiddleTable();
    TwiddleTable(int initial_size = 14);

    const PerSizeTables<Context>& operator[](int k) const;
    void ensure(int k);

private:
    void expand(int k);

private:
    std::atomic<int> m_size_k;
    SpinLock m_resize_lock;
    PerSizeTables<Context> m_tables[32];
};



template <typename Context>
struct TableRowReader{
    using vtype = typename Context::vtype;

    const vcomplex<Context>* data;

    TableRowReader(const vcomplex<Context>* p_data)
        : data(p_data)
    {}


    PA_FORCE_INLINE void get(float& real, float& imag, size_t index) const{
        const vcomplex<Context>& point = data[index / Context::VECTOR_LENGTH];
        size_t sindex = index % Context::VECTOR_LENGTH;
        real = point.real(sindex);
        imag = point.imag(sindex);
    }
#if 0
    PA_FORCE_INLINE void get(vtype& real, vtype& imag, size_t index) const{
        const vcomplex<Context>& point = data[index];
        real = point.r;
        imag = point.i;
    }
#endif

};



}
}
}
#endif
