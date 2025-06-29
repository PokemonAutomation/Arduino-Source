/*  Image Filters Basic Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stdint.h>
#include "Common/Compiler.h"

namespace PokemonAutomation{
namespace Kernels{





template <typename PixelTester>
class FilterImage_Rgb32_Default{
public:
    static const size_t VECTOR_SIZE = 1;
    using Mask = size_t;

public:
    FilterImage_Rgb32_Default(
        const PixelTester& tester,
        uint32_t replacement_color, bool replace_color_within_range
    )
        : m_tester(tester)
        , m_replacement_color(replacement_color)
        , m_replace_color_within_range(replace_color_within_range)
        , m_count(0)
    {}

    PA_FORCE_INLINE size_t count() const{
        return m_count;
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        uint32_t pixel = in[0];
        bool passed = m_tester.test_word(pixel);
        m_count += passed;
        passed ^= m_replace_color_within_range;
        out[0] = passed ? pixel : m_replacement_color;
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, size_t left){
        process_full(out, in);
    }

private:
    const PixelTester m_tester;
    const uint32_t m_replacement_color;
    const bool m_replace_color_within_range;
    size_t m_count;
};






template <typename PixelTester>
class ToBlackWhite_Rgb32_Default{
public:
    static const size_t VECTOR_SIZE = 1;
    using Mask = size_t;

public:
    ToBlackWhite_Rgb32_Default(
        const PixelTester& tester,
        bool in_range_black
    )
        : m_tester(tester)
        , m_in_range_black(in_range_black ? 1 : 0)
        , m_count(0)
    {}

    PA_FORCE_INLINE size_t count() const{
        return m_count;
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        uint32_t pixel = in[0];
        bool passed = m_tester.test_word(pixel);
        m_count += passed;
        passed ^= m_in_range_black;
        out[0] = passed ? 0xffffffff : 0xff000000;
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, size_t left){
        process_full(out, in);
    }

private:
    const PixelTester m_tester;
    const bool m_in_range_black;
    size_t m_count;
};




}
}
