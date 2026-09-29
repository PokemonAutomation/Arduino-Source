/*  Xorshift128
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_Xorshift128_H
#define PokemonAutomation_Pokemon_Xorshift128_H

#include <stddef.h>
#include <stdint.h>
#include <array>
#include <string>
#include "Pokemon_Gf2Matrix.h"

namespace PokemonAutomation{
namespace Pokemon{


struct Xorshift128State{
    uint32_t s0 = 0;
    uint32_t s1 = 0;
    uint32_t s2 = 0;
    uint32_t s3 = 0;

    Xorshift128State() = default;
    Xorshift128State(uint32_t p_s0, uint32_t p_s1, uint32_t p_s2, uint32_t p_s3)
        : s0(p_s0), s1(p_s1), s2(p_s2), s3(p_s3)
    {}

    bool operator==(const Xorshift128State& x) const{
        return s0 == x.s0 && s1 == x.s1 && s2 == x.s2 && s3 == x.s3;
    }
    bool operator!=(const Xorshift128State& x) const{ return !(*this == x); }

    //  "[0x........, 0x........, 0x........, 0x........]"
    std::string to_string() const;
};


Xorshift128State xorshift128_state_from_seed_pair(uint64_t seed0, uint64_t seed1);
void xorshift128_state_to_seed_pair(const Xorshift128State& state, uint64_t& seed0, uint64_t& seed1);


Gf2Vec128 xorshift128_state_to_vector(const Xorshift128State& state);
Xorshift128State xorshift128_state_from_vector(const Gf2Vec128& vector);


class Xorshift128{
public:
    Xorshift128() = default;
    explicit Xorshift128(const Xorshift128State& state) : m_state(state) {}
    Xorshift128(uint32_t s0, uint32_t s1, uint32_t s2, uint32_t s3) : m_state(s0, s1, s2, s3) {}

    const Xorshift128State& state() const{ return m_state; }
    void set_state(const Xorshift128State& state){ m_state = state; }

    uint32_t next();

    //  Step backwards. Undoes exactly one next().
    void prev();

    void advance(uint64_t count);
    void rewind(uint64_t count);

private:
    Xorshift128State m_state;
};


const Gf2Matrix128& xorshift128_transition_matrix();
const Gf2Matrix128& xorshift128_inverse_transition_matrix();

Gf2Matrix128 xorshift128_transition_power(uint64_t count);


inline uint32_t bdsp_gen_transform(uint32_t raw){
    return (uint32_t)(raw % 0xFFFFFFFF) + 0x80000000;
}


//  A sliding window of pre-generated outputs. Size must be a power of two.
template <size_t Size>
class Xorshift128List{
    static_assert(Size != 0 && (Size & (Size - 1)) == 0, "Size must be a power of two.");

public:
    explicit Xorshift128List(const Xorshift128& rng)
        : m_rng(rng)
    {
        for (uint32_t& value : m_buffer){
            value = m_rng.next();
        }
    }

    uint32_t next_raw(){ return m_buffer[m_index++ & (Size - 1)]; }
    uint32_t next_gen(){ return bdsp_gen_transform(next_raw()); }

    uint32_t next_raw_modulo(uint32_t modulo){ return next_raw() % modulo; }
    uint32_t next_gen_modulo(uint32_t modulo){ return next_gen() % modulo; }

    //  Skip values without reading them.
    void advance(size_t count){ m_index += count; }

    void advance_state(){
        m_buffer[m_head++ & (Size - 1)] = m_rng.next();
        m_index = m_head;
    }

    //  Rewind the read position to the start of the current window.
    void reset_index(){ m_index = m_head; }

private:
    Xorshift128 m_rng;
    std::array<uint32_t, Size> m_buffer;
    size_t m_head = 0;
    size_t m_index = 0;
};


}
}
#endif
