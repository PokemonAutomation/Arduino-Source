/*  SIMD Debuggers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SIMDDebuggers_H
#define PokemonAutomation_SIMDDebuggers_H

#include <stdint.h>
#include <emmintrin.h>

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

inline void print8(__m128i x){
    union{
        __m128i v;
        uint8_t s[16];
    };
    v = x;
    bool first = true;
    for (int c = 0; c < 16; c++){
        if (!first){
            cout << " ";
        }
        first = false;
        cout << (unsigned)s[c];
    }
}
inline void print16(__m128i x){
    union{
        __m128i v;
        uint16_t s[8];
    };
    v = x;
    cout << s[0] << " " << s[1] << " " << s[2] << " " << s[3] << " " << s[4] << " " << s[5] << " " << s[6] << " " << s[7];
}
inline void print32(__m128i x){
    union{
        __m128i v;
        uint32_t s[4];
    };
    v = x;
    cout << s[0] << " " << s[1] << " " << s[2] << " " << s[3];
}


}
#endif
