/*  Partial Word Access (arm64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_PartialWordAccess_arm64_NEON_H
#define PokemonAutomation_Kernels_PartialWordAccess_arm64_NEON_H

#include <stdint.h>
#include <cstddef>
#include <arm_neon.h>
#include "Common/Compiler.h"

namespace PokemonAutomation{
namespace Kernels{


// Used to load bytes of data to partially fill a SIMD 128-bit vector
// This is useful for handling leftover bytes of data at the end of a loop.
// Example usage: Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_arm64_NEON.h
// 
class PartialWordAccess_arm64_NEON{
public:
    // create a mask with first `bytes` low bytes are all 1s
    // If `bytes` is 3, then the returned vector is from low bytes to high bytes: [0xFF, 0xFF, 0xFF, 0, 0, 0, ..., 0]
    PA_FORCE_INLINE static uint8x16_t create_front_mask(size_t bytes) {
        PA_ALIGN_STRUCT(16) uint8_t bytes_values[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
        const uint8x16_t seq_u8x16 = vld1q_u8(bytes_values);
        return vcgtq_u8(vdupq_n_u8((uint8_t)bytes), seq_u8x16);
    }

    // How many bytes of data to load. Allow at most 16 bytes.
    PA_FORCE_INLINE PartialWordAccess_arm64_NEON(size_t bytes)
        : m_shift(16 - bytes)
    {
        PA_ALIGN_STRUCT(16) uint8_t bytes_values[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
        const uint8x16_t seq_u8x16 = vld1q_u8(bytes_values);
        // If `bytes` is 3, then `m_front_mask` is from low bytes to high bytes: [0xFF, 0xFF, 0xFF, 0, 0, 0, ..., 0]
        m_front_mask = vcgtq_u8(vdupq_n_u8((uint8_t)bytes), seq_u8x16);
        // If `bytes` is 3, then `m_back_mask` is from low bytes to high bytes: [0xFF, 0xFF, 0xFF, ... 0xFF, 0, 0, 0]
        m_back_mask = vcgtq_u8(vdupq_n_u8((uint8_t)m_shift), seq_u8x16);

        // If `bytes` is 3, then `m_shift_front` is from low bytes to high bytes: [13, 14, 15, 16, 17, ..., 28]
        m_shift_front = vaddq_u8(vdupq_n_u8(uint8_t(m_shift)), seq_u8x16);

        // IF `bytes` is 3, then `m_shift_back` is from low bytes to high bytes: [243, 244, 245,... 0, 1, 2]
        m_shift_back = vsubq_u8(seq_u8x16, vdupq_n_u8((uint8_t)m_shift));
    }

    // load() function that does not read past end of buffer
    // so it read some in front of `ptr`
    // return the loaded partial word data and store at lower bytes of returned uint8x16_t
    PA_FORCE_INLINE uint8x16_t load_int_no_read_past_end(const void* ptr) const{
        // In `x` the actual data we want, `ptr` to `ptr + bytes`, is at the higher bits
        uint8x16_t x = vld1q_u8((const uint8_t*)ptr - m_shift);
        // Use a table lookup command:
        // for each uint8 in the result, ret_u8[i], get the index from `m_shift_front`: m_shift_front[i]
        // use the value of m_shift_front[i] as an index to get a value in x:
        // ret_u8[i] = x[m_shift_front[i]]
        // since `m_shift_front` stores [`16-bytes`, `16-bytes+1`, `16-bytes+2`, ...]
        // the resulting operation is to shift the bytes in x to the lower bytes by `16-bytes` bytes.
        // For the index values >= 16 in m_shift_front[i], `vqtbl1q_u8()` returns 0.
        return vqtbl1q_u8(x, m_shift_front);
    }
    // load() function that reads past end of buffer
    // return the loaded partial word data and store at lower bytes of returned uint8x16_t
    PA_FORCE_INLINE uint8x16_t load_int_no_read_before_ptr(const void* ptr) const{
        uint8x16_t x = vld1q_u8((const uint8_t*)ptr);
        return vandq_u8(x, m_front_mask);
    }
    // load() function that does not read past end of buffer
    // so it read some in front of `ptr`
    // return the loaded partial word data and store at lower bytes of returned float32x4_t
    PA_FORCE_INLINE float32x4_t load_f32_no_read_past_end(const void* ptr) const{
        return vreinterpretq_f32_u8(load_int_no_read_past_end(ptr));
    }
    // load() function that reads past end of buffer
    // return the loaded partial word data and store at lower bytes of returned float32x4_t
    PA_FORCE_INLINE float32x4_t load_f32_no_read_before_ptr(const void* ptr) const{
        return vreinterpretq_f32_u8(load_int_no_read_past_end(ptr));
    }

    // Load only `bytes` of bytes from `ptr` and place them at the lower end of of the
    // returned uint8x16_t.
    // `bytes` is defined at the constructor of `PartialWordAccess_arm64_NEON`.
    //
    // Note: to be efficient, we read an entire 16-byte memory on this partial word.
    // This is a hack as it may read outside of the memory we allocate to the buffer of
    // `ptr`, but we assume here the buffer is defined inside our pre-allocated
    // 4K-bytes-aligned memory block. So as long as we don't read past a 4K byte page,
    // we are fine.
    // Note OS allocates memory by groups of 4K bytes, too. So it could still work fine
    // when not run on our pre-allocated memory block.
    PA_FORCE_INLINE uint8x16_t load(const void* ptr) const{
        const void* end = (const char*)ptr + 16;
        if (((size_t)end & 4095) < 16){ // if we are about to read past a page
            // Call load function that does not read past page
            return load_int_no_read_past_end(ptr);
        }else{
            // Call load function that reads past end of the data buffer where `ptr` is
            // from.
            return load_int_no_read_before_ptr(ptr);
        }
    }

    // store() function that does not read past end of buffer
    // so it read some in front of `ptr` and save them back into buffer
    PA_FORCE_INLINE void store_int_no_past_end(void* ptr, uint8x16_t x) const{
        uint8x16_t v = vld1q_u8((const uint8_t*)ptr - m_shift);
        // use table lookup to shift bytes in x to higher bytes by `bytes`
        x = vqtbl1q_u8(x, m_shift_back);
        // bit select intrinsic: 
        // vbslq_u8(a, b, c), for 1-bits in a, choose b; for 0-bits in a, choose c
        // Returned `merged` contains the pre-ptr data in the buffer, and partial word in `x`.
        uint8x16_t merged = vbslq_u8(m_back_mask, v, x);
        vst1q_u8((uint8_t*)ptr - m_shift, merged);
    }
    // store() function that does not read past end of buffer
    // so it read some in front of `ptr` and save them back into buffer
    PA_FORCE_INLINE void store_f32_no_past_end(void* ptr, float32x4_t x) const{
        store_int_no_past_end(ptr, vreinterpretq_u8_f32(x));
    }

private:
    size_t m_shift;
    // mask of which bytes are occupied by the loaded data
    uint8x16_t m_front_mask;
    uint8x16_t m_back_mask;
    uint8x16_t m_shift_front;
    uint8x16_t m_shift_back;
};



}
}
#endif
