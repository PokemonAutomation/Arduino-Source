/*  Kernels (arm64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_arm64_NEON_H
#define PokemonAutomation_Kernels_arm64_NEON_H

#include <stdint.h>
#include <arm_neon.h>
#include "Common/Compiler.h"

#include <iostream>

namespace PokemonAutomation{
namespace Kernels{

// Cout the four floats in float32x4_t, separated by " "
inline static void print(const float32x4_t& x){
    union{
        float32x4_t v;
        float s[4];
    };
    v = x;
    for (int i = 0; i < 4; i++){
        std::cout << s[i] << " ";
    }
    std::cout << std::endl;
}

// Cout the 16 uint8 in uint8x16_t, separated by " "
inline static void print_u8(uint8x16_t x){
    for (int i = 0; i < 16; i++){
        std::cout << (int)x[i] << " ";
    }
    std::cout << std::endl;
}

// Cout the 8 uint16 in uint16x8_t, separated by " "
inline static void print_u16(const uint16x8_t& x){
    union{
        uint16x8_t v;
        uint16_t s[8];
    };
    v = x;
    for (int i = 0; i < 8; i++){
        std::cout << s[i] << " ";
    }
    std::cout << std::endl;
}

// Cout the 4 uint32 in uint32x4_t, separated by " "
inline static void print_u32(const uint32x4_t& x){
    union{
        uint32x4_t v;
        uint32_t s[4];
    };
    v = x;
    for (int i = 0; i < 4; i++){
        std::cout << s[i] << " ";
    }
    std::cout << std::endl;
}

// Cout the 2 uint64 in uint64x2_t, separated by " "
inline static void print_u64(const uint64x2_t& x){
    union{
        uint64x2_t v;
        uint64_t s[2];
    };
    v = x;
    for (int i = 0; i < 2; i++){
        std::cout << s[i] << " ";
    }
    std::cout << std::endl;
}


// Add all four uint32 together
PA_FORCE_INLINE uint64_t reduce32_arm64_NEON(uint32x4_t x){
    uint64_t ret = vgetq_lane_u32(x, 0);
    ret += vgetq_lane_u32(x, 1);
    ret += vgetq_lane_u32(x, 2);
    ret += vgetq_lane_u32(x, 3);
    return ret;
}

// Create a new uint64x2_t vector from the lower half of r0 and r1 (r1 portion on higher resulting bits),
// and one new uint64x2_t vector from the upper half of them.
// Assign r0 to the former, r1 to the letter.
PA_FORCE_INLINE void transpose_u64_2x2_NEON(uint64x2_t& r0, uint64x2_t& r1){
    int64x1_t r0_l = vget_low_u64(r0);
    int64x1_t r1_l = vget_low_u64(r1);
    int64x1_t r0_h = vget_high_u64(r0);
    int64x1_t r1_h = vget_high_u64(r1);

    r0 = vcombine_u64(r0_l, r1_l);
    r1 = vcombine_u64(r0_h, r1_h);
}




}
}
#endif
