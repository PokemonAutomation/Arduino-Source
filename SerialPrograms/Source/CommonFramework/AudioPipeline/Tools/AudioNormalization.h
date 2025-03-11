/*  Audio Normalization
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonFramework_AudioPipeline_AudioNormalization_H
#define PokemonAutomation_CommonFramework_AudioPipeline_AudioNormalization_H

#include <stdint.h>
#include <limits>
#include <type_traits>
#include "Common/Compiler.h"

#if _WIN32
#include <stdlib.h>
#endif

namespace PokemonAutomation{


PA_FORCE_INLINE uint8_t byte_swap(uint8_t x){
    return x;
}
PA_FORCE_INLINE int8_t byte_swap(int8_t x){
    return x;
}
PA_FORCE_INLINE uint16_t byte_swap(uint16_t x){
#if _WIN32
    return (uint32_t)_byteswap_ulong(x) >> 16;
#elif  __GNUC__
    return (uint32_t)__builtin_bswap32(x) >> 16;
#else
#error "No bswap() intrinsic for this compiler."
#endif
}
PA_FORCE_INLINE int16_t byte_swap(int16_t x){
#if _WIN32
    return (int16_t)((uint32_t)_byteswap_ulong(x) >> 16);
#elif  __GNUC__
    return (int16_t)((uint32_t)__builtin_bswap32(x) >> 16);
#else
#error "No bswap() intrinsic for this compiler."
#endif
}
PA_FORCE_INLINE uint32_t byte_swap(uint32_t x){
#if _WIN32
    return _byteswap_ulong(x);
#elif  __GNUC__
    return __builtin_bswap32(x);
#else
#error "No bswap() intrinsic for this compiler."
#endif
}
PA_FORCE_INLINE int16_t byte_swap(int32_t x){
#if _WIN32
    return (int16_t)_byteswap_ulong(x);
#elif  __GNUC__
    return __builtin_bswap32(x);
#else
#error "No bswap() intrinsic for this compiler."
#endif
}


template <typename Type>
void normalize_audio_le(float* out, const Type* in, size_t count){
    const float rcp = (std::is_unsigned<Type>::value ? 2.0f : 1.0f) / (float)std::numeric_limits<Type>::max();
    const float sub = std::is_unsigned<Type>::value ? 1.0f : 0.0f;
    for (size_t c = 0; c < count; c++){
        out[c] = (float)in[c] * rcp - sub;
    }
}

template <typename Type>
void normalize_audio_be(float* out, const Type* in, size_t count){
    const float rcp = (std::is_unsigned<Type>::value ? 2.0f : 1.0f) / (float)std::numeric_limits<Type>::max();
    const float sub = std::is_unsigned<Type>::value ? 1.0f : 0.0f;
    for (size_t c = 0; c < count; c++){
        out[c] = (float)byte_swap(in[c]) * rcp - sub;
    }
}


}
#endif
