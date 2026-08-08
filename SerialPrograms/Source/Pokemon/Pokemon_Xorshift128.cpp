/*  Xorshift128
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Pokemon_Xorshift128.h"

namespace PokemonAutomation{
namespace Pokemon{


const uint64_t JUMP_THRESHOLD = 512;


std::string Xorshift128State::to_string() const{
    return "[0x" + tostr_hex_padded(8, s0)
        + ", 0x" + tostr_hex_padded(8, s1)
        + ", 0x" + tostr_hex_padded(8, s2)
        + ", 0x" + tostr_hex_padded(8, s3) + "]";
}


Xorshift128State xorshift128_state_from_seed_pair(uint64_t seed0, uint64_t seed1){
    return Xorshift128State(
        (uint32_t)(seed0 >> 32), (uint32_t)seed0,
        (uint32_t)(seed1 >> 32), (uint32_t)seed1
    );
}
void xorshift128_state_to_seed_pair(const Xorshift128State& state, uint64_t& seed0, uint64_t& seed1){
    seed0 = ((uint64_t)state.s0 << 32) | state.s1;
    seed1 = ((uint64_t)state.s2 << 32) | state.s3;
}

Gf2Vec128 xorshift128_state_to_vector(const Xorshift128State& state){
    Gf2Vec128 ret;
    xorshift128_state_to_seed_pair(state, ret.high, ret.low);
    return ret;
}
Xorshift128State xorshift128_state_from_vector(const Gf2Vec128& vector){
    return xorshift128_state_from_seed_pair(vector.high, vector.low);
}



uint32_t Xorshift128::next(){
    uint32_t temp = m_state.s0 ^ (m_state.s0 << 11);
    uint32_t old_s3 = m_state.s3;

    m_state.s0 = m_state.s1;
    m_state.s1 = m_state.s2;
    m_state.s2 = m_state.s3;
    m_state.s3 = temp ^ (temp >> 8) ^ old_s3 ^ (old_s3 >> 19);

    return m_state.s3;
}

void Xorshift128::prev(){
    uint32_t temp = (m_state.s2 >> 19) ^ m_state.s2 ^ m_state.s3;

    temp ^= temp >> 8;
    temp ^= temp >> 16;

    temp ^= temp << 11;
    temp ^= temp << 22;

    m_state.s3 = m_state.s2;
    m_state.s2 = m_state.s1;
    m_state.s1 = m_state.s0;
    m_state.s0 = temp;
}

void Xorshift128::advance(uint64_t count){
    if (count < JUMP_THRESHOLD){
        for (uint64_t c = 0; c < count; c++){
            next();
        }
        return;
    }
    m_state = xorshift128_state_from_vector(
        xorshift128_transition_power(count) * xorshift128_state_to_vector(m_state)
    );
}
void Xorshift128::rewind(uint64_t count){
    if (count < JUMP_THRESHOLD){
        for (uint64_t c = 0; c < count; c++){
            prev();
        }
        return;
    }
    m_state = xorshift128_state_from_vector(
        xorshift128_inverse_transition_matrix().pow(count) * xorshift128_state_to_vector(m_state)
    );
}



template <typename StepFunction>
static Gf2Matrix128 build_step_matrix(StepFunction&& step){
    Gf2Matrix128 matrix;
    for (size_t column = 0; column < 128; column++){
        Gf2Vec128 basis;
        basis.set(column, true);

        Xorshift128 rng(xorshift128_state_from_vector(basis));
        step(rng);
        Gf2Vec128 image = xorshift128_state_to_vector(rng.state());

        for (size_t row = 0; row < 128; row++){
            if (image.get(row)){
                matrix[row].set(column, true);
            }
        }
    }
    return matrix;
}

const Gf2Matrix128& xorshift128_transition_matrix(){
    static Gf2Matrix128 matrix = build_step_matrix([](Xorshift128& rng){ rng.next(); });
    return matrix;
}
const Gf2Matrix128& xorshift128_inverse_transition_matrix(){
    static Gf2Matrix128 matrix = build_step_matrix([](Xorshift128& rng){ rng.prev(); });
    return matrix;
}

//  T^(2^k) for every k that fits in a 64-bit count.
static const std::array<Gf2Matrix128, 64>& xorshift128_transition_powers_of_two(){
    static std::array<Gf2Matrix128, 64> table = [](){
        std::array<Gf2Matrix128, 64> ret;
        ret[0] = xorshift128_transition_matrix();
        for (size_t c = 1; c < ret.size(); c++){
            ret[c] = ret[c - 1] * ret[c - 1];
        }
        return ret;
    }();
    return table;
}

Gf2Matrix128 xorshift128_transition_power(uint64_t count){
    const std::array<Gf2Matrix128, 64>& powers = xorshift128_transition_powers_of_two();
    Gf2Matrix128 ret = Gf2Matrix128::identity();
    for (size_t bit = 0; count != 0; count >>= 1, bit++){
        if ((count & 1) != 0){
            ret = powers[bit] * ret;
        }
    }
    return ret;
}




}
}
