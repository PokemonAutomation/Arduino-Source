/*  Xoroshiro128+ and reverse
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Xoroshiro128Plus_H
#define PokemonAutomation_PokemonSwSh_Xoroshiro128Plus_H

#include <stdint.h>
#include <utility>
#include <vector>

namespace PokemonAutomation{
namespace Pokemon{

struct Xoroshiro128PlusState{
    Xoroshiro128PlusState(uint64_t s0, uint64_t s1);
    uint64_t s0;
    uint64_t s1;
};

class Xoroshiro128Plus{
public:
    Xoroshiro128PlusState state;

    Xoroshiro128Plus(Xoroshiro128PlusState state);
    Xoroshiro128Plus(uint64_t s0, uint64_t s1);
    uint64_t next();
    uint64_t nextInt(uint64_t);
    Xoroshiro128PlusState get_state();
    std::vector<bool> generate_last_bit_sequence(size_t max_advances);

    // Calculates how many advances are required to reach the given state.
    // The given state must be reachable within max_advances advances.
    // Returns a pair:
    // first: true if the state is reachable within max_advances, false otherwise
    // second: the number of advances required (if first is true)
    std::pair<bool, uint64_t> advances_to_state(Xoroshiro128PlusState other_state, uint64_t max_advances = 100000);

    static Xoroshiro128Plus xoroshiro128plus_from_last_bits(std::pair<uint64_t, uint64_t> last_bits);


private:
    static uint64_t last_bits_reverse_matrix[128][2];

    uint64_t rotl(const uint64_t x, int k);
};

}
}
#endif
