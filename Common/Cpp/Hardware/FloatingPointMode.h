/*  Floating-Point Mode
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Turn on flush-to-zero for denormal floats on the current thread.
 *
 *  Denormal (subnormal) floats are nonzero values too small for the normal float
 *  format: below about 1.2e-38 for float. Intel CPUs handle many operations on one
 *  in microcode, which is many times slower than normal float math. AMD and Apple
 *  CPUs pay much less. The large PaddleOCR model for Chinese and Japanese produces
 *  them: on the Mac Intel CI runner (an i7-8700B), each of those OCR tests took
 *  35-185 s, and under 5 s with flush-to-zero.
 *
 *  With flush-to-zero, the CPU treats denormal inputs as zero and rounds denormal
 *  results to zero. OCR, image matching and model inference don't need values
 *  that small.
 *
 *  The mode belongs to each thread:
 *  - x86-64: the FTZ and DAZ bits of MXCSR, which control SSE and AVX math.
 *  - ARM64 (GCC and Clang): the FZ bit of FPCR, which controls scalar and NEON math.
 *  - Anywhere else, these functions do nothing.
 *
 */

#ifndef PokemonAutomation_Hardware_FloatingPointMode_H
#define PokemonAutomation_Hardware_FloatingPointMode_H

#include <stdint.h>
#if defined(__x86_64__) || defined(_M_X64)
#include <xmmintrin.h>
#endif

namespace PokemonAutomation{


// Return the current thread's floating-point control register, so it can be
// restored later with `set_floating_point_control()`.
inline uint64_t get_floating_point_control(){
#if defined(__x86_64__) || defined(_M_X64)
    return _mm_getcsr();
#elif defined(__aarch64__)
    uint64_t fpcr;
    __asm__ __volatile__("mrs %0, fpcr" : "=r"(fpcr));
    return fpcr;
#else
    return 0;
#endif
}

// Restore a value returned by `get_floating_point_control()`.
inline void set_floating_point_control(uint64_t value){
#if defined(__x86_64__) || defined(_M_X64)
    _mm_setcsr((unsigned int)value);
#elif defined(__aarch64__)
    __asm__ __volatile__("msr fpcr, %0" : : "r"(value));
#else
    (void)value;
#endif
}

// Turn on flush-to-zero for denormal floats on the current thread, until the
// thread ends or the mode is changed again.
inline void enable_flush_denormals_to_zero(){
#if defined(__x86_64__) || defined(_M_X64)
    //  0x8000: FTZ, flush denormal results to zero.
    //  0x0040: DAZ, treat denormal inputs as zero.
    _mm_setcsr(_mm_getcsr() | 0x8040);
#elif defined(__aarch64__)
    //  Bit 24: FZ, flush denormal inputs and results to zero.
    set_floating_point_control(get_floating_point_control() | ((uint64_t)1 << 24));
#endif
}


// Turn on flush-to-zero on the current thread for this object's lifetime, then
// restore the previous mode. For code that may run on threads this program
// doesn't own, like an ONNX Runtime `Run()` call: ONNX Runtime runs part of
// each inference on the calling thread.
class FlushDenormalsToZeroScope{
public:
    FlushDenormalsToZeroScope(const FlushDenormalsToZeroScope&) = delete;
    void operator=(const FlushDenormalsToZeroScope&) = delete;

    FlushDenormalsToZeroScope()
        : m_saved(get_floating_point_control())
    {
        enable_flush_denormals_to_zero();
    }
    ~FlushDenormalsToZeroScope(){
        set_floating_point_control(m_saved);
    }

private:
    uint64_t m_saved;
};


}
#endif
