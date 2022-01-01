/*  Test Program (Computer)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Tile_Default.h"
//#include "Kernels/Waterfill/Kernels_Waterfill_Tile_x64_SSE41.h"
//#include "Kernels/Waterfill/Kernels_Waterfill_Tile_x64_AVX2.h"
//#include "Kernels/Waterfill/Kernels_Waterfill_Tile_x64_AVX512.h"
//#include "Kernels/Kernels_x64_AVX512.h"
#include "TestProgramComputer.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


TestProgramComputer_Descriptor::TestProgramComputer_Descriptor()
    : RunnableComputerProgramDescriptor(
        "Computer:TestProgram",
        "Computer", "Test Program (Computer)",
        "",
        "Test Program"
    )
{}



TestProgramComputer::TestProgramComputer(const TestProgramComputer_Descriptor& descriptor)
    : RunnableComputerProgramInstance(descriptor)
{
}


#if 0
inline static void print_u64(__m128i x){
    for (int i = 0; i < 2; i++){
        std::cout << x.m128i_u64[i] << " ";
    }
    std::cout << std::endl;
}
inline static void print_u64(__m256i x){
    for (int i = 0; i < 4; i++){
        std::cout << x.m256i_u64[i] << " ";
    }
    std::cout << std::endl;
}
#endif
#if 0
inline static void print_u8(__m128i x){
    for (int i = 0; i < 16; i++){
        std::cout << (int)((const unsigned char*)&x)[i] << " ";
    }
    std::cout << std::endl;
}
inline static void print_u64(const __m512i& x){
    union{
        __m512i v;
        uint64_t s[8];
    };
    v = x;
    for (int i = 0; i < 8; i++){
        std::cout << s[i] << " ";
    }
    std::cout << std::endl;
}
inline static void print_u8(__m512i x){
    for (int i = 0; i < 64; i++){
        std::cout << (int)((const unsigned char*)&x)[i] << " ";
    }
    std::cout << std::endl;
}
#endif

inline std::string dump8(uint8_t x){
    std::string str;
    for (size_t c = 0; c < 8; c++){
        str += ((x >> c) & 1) ? "1" : "0";
    }
    return str;
}



void TestProgramComputer::program(ProgramEnvironment& env){
    using namespace Kernels;

#if 0
    __m512i r0 = _mm512_set1_epi64(0x0123456789abcdef);
    print_u64(bit_reverse0(r0));
    print_u64(bit_reverse0(r0));
#endif

#if 0
    __m128i r0 = _mm_set1_epi64x(0x3b);
    cout << dump8(*(uint8_t*)&r0) << endl;

    r0 = _mm_gf2p8affine_epi64_epi8(r0, _mm_set1_epi64x(0x8040201008040201), 0);
//    r0 = _mm_gf2p8affine_epi64_epi8(r0, _mm_set1_epi64x(0x0102040810204080), 0);

    cout << dump8(*(uint8_t*)&r0) << endl;
#endif

#if 0
    BinaryTile_AVX512 m, x;
    m.set_zero();
    x.set_zero();
    for (size_t c = 0; c < 64; c++){
        m.row(c) = -1;
    }
    m.row( 0) = 0b1111111011111011;
    m.row( 1) = 0b1110111011111011;
    m.row( 2) = 0b1011111011011111;
    m.row( 3) = 0b1110101011010011;
    m.row( 4) = 0b1111111001001011;
    m.row( 8) = 0b1111111101011011;
//    m.row(10) = 0;
    m.row(12) = 0b1111110111011111;
    cout << m.dump() << endl;
    cout << "-------------" << endl;

    x.row(63) = 0b1000000000000000;

    waterfill_expand(m, x);
    cout << x.dump() << endl;
#endif


#if 0
    __m512i r0 = _mm512_setr_epi64(10, 11, 12, 13, 14, 15, 16, 17);

    print_u64(r0);
    r0 = _mm512_permutexvar_epi64(_mm512_set1_epi64(7), r0);
    print_u64(r0);
#endif

#if 0
    {
        __m512i r0 = _mm512_setr_epi64(0, 0, 0, 0, 1, 1, 1, 1);
        __m512i r1 = _mm512_setr_epi64(0, 0, 1, 1, 0, 0, 1, 1);
        __m512i r2 = _mm512_setr_epi64(0, 1, 0, 1, 0, 1, 0, 1);

        __m512i s, x;
        s = _mm512_xor_si512(r1, r2);
        s = _mm512_and_si512(s, r2);
        x = _mm512_or_si512(r0, s);

        print_u64(x);
    }
    {
        __m512i r0 = _mm512_setr_epi64(0, 0, 0, 0, 1, 1, 1, 1);
        __m512i r1 = _mm512_setr_epi64(0, 0, 1, 1, 0, 0, 1, 1);
        __m512i r2 = _mm512_setr_epi64(0, 1, 0, 1, 0, 1, 0, 1);

        r0 = _mm512_ternarylogic_epi64(r0, r1, r2, 0b11110010);

        print_u64(r0);
    }
#endif


#if 0
    __m512i x;
    uint8_t s[64];
    for (size_t c = 0; c < 64; c++){
        s[c] = (uint8_t)c;
    }
    memcpy(&x, s, 64);

    print_u8(x);
    x = _mm512_shuffle_epi8(
        x,
        _mm512_setr_epi8(
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8
        )
    );
    print_u8(x);
#endif

#if 0
    __m256i x = _mm256_setr_epi64x(10, 11, 12, 13);
    print_u64(x);
    print_u64(_mm256_permute4x64_epi64(x, 255));
#endif

#if 0
    BinaryTile_AVX2 m, x;
    m.set_zero();
    x.set_zero();
    for (size_t c = 0; c < 16; c++){
        m.row(c) = -1;
    }
    m.row( 0) = 0b1111111011111011;
    m.row( 1) = 0b1110111011111011;
    m.row( 2) = 0b1011111011011111;
    m.row( 3) = 0b1110101011010011;
    m.row( 4) = 0b1111111001001011;
    m.row( 8) = 0b1111111101011011;
//    m.row(10) = 0;
    m.row(12) = 0b1111110111011111;
    cout << m.dump() << endl;
    cout << "-------------" << endl;

    x.row(15) = 0b1000000000000000;

    waterfill_expand(m, x);
    cout << x.dump() << endl;
#endif
#if 0
    BinaryTile_SSE41 m, x;
    m.set_zero();
    x.set_zero();
#if 0
    m.row( 0) = 1;
    m.row( 1) = 1;
    m.row( 2) = 1;
    m.row( 3) = 0;
    m.row( 4) = 1;
    m.row( 5) = 1;
    m.row( 6) = 1;
    m.row( 7) = 1;
#endif
#if 1
    for (size_t c = 0; c < 8; c++){
        m.row(c) = -1;
    }
    m.row( 0) = 0b1111111011111011;
    m.row( 1) = 0b1110111011111011;
    m.row( 2) = 0b1011111011011111;
    m.row( 3) = 0b1110101011010011;
    m.row( 4) = 0b1111111001001011;
//    m.row( 5) = 0;
#endif
    cout << m.dump() << endl;
    cout << "-------------" << endl;

//    x.row(3) = 1;
    x.row(7) = 0b1000000000000000;

    waterfill_expand(m, x);
    cout << x.dump() << endl;
#endif

//    __m256i v = _mm256_set1_epi64x(123);
//    cout << _mm256_testz_si256(v, v) << endl;


#if 0
    __m128i x = _mm_set_epi64x(10, 20);
    print_u64(x);
    print_u64(_mm_shuffle_epi32(x, 78));
#endif

#if 0
    BinaryTile_Default m, x;
    m.set_zero();
    x.set_zero();
    m.row(0) = 0b1111111011111011;
    m.row(1) = 0b1111111001001011;
    m.row(2) = 0b1111111101011011;
    m.row(3) = 0b1111110111011111;
    cout << m.dump() << endl;

    x.row(0) = 0b1000000000000000;

    waterfill_expand(m, x);
    cout << x.dump() << endl;
#endif


#if 0
    uint64_t m0 = 0b11011110;
    uint64_t x0 = 0b00000011;
    expand_forward(x0, m0);
    print64(m0);
    print64(x0);
#endif

#if 0
    uint64_t m0 = 0b11011110;
    uint64_t m1 = 0b11001111;
    uint64_t x0 = 0b00000010;
    uint64_t x1 = 0b00000000;

    print64(m0);
    print64(m1);
    print64(x0);
    print64(x1);

    expand(x0, m0);
    touch_vertical(x1, x0, m1);

    print64(m0);
    print64(m1);
    print64(x0);
    print64(x1);
#endif

}








}
