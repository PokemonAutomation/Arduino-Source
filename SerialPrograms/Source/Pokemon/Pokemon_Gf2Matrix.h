/*  GF(2) Linear Algebra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_Gf2Matrix_H
#define PokemonAutomation_Pokemon_Gf2Matrix_H

#include <stddef.h>
#include <stdint.h>
#include <array>
#include <string>
#include <vector>

namespace PokemonAutomation{
namespace Pokemon{


//  A 128-bit vector over GF(2).
//  Index 0 is the MSB of "high". Index 127 is the LSB of "low".
struct Gf2Vec128{
    uint64_t high = 0;
    uint64_t low = 0;

    Gf2Vec128() = default;
    Gf2Vec128(uint64_t p_high, uint64_t p_low) : high(p_high), low(p_low) {}

    bool operator==(const Gf2Vec128& x) const{ return high == x.high && low == x.low; }
    bool operator!=(const Gf2Vec128& x) const{ return !(*this == x); }

    bool get(size_t index) const;
    void set(size_t index, bool value);

    bool is_zero() const{ return (high | low) == 0; }

    //  Parity of the bitwise AND. This is the GF(2) dot product.
    bool dot(const Gf2Vec128& x) const;

    Gf2Vec128 operator^(const Gf2Vec128& x) const{ return Gf2Vec128(high ^ x.high, low ^ x.low); }
    Gf2Vec128& operator^=(const Gf2Vec128& x){ high ^= x.high; low ^= x.low; return *this; }
};


//  A 128x128 matrix over GF(2), stored as 128 row vectors.
//  Vectors are treated as columns, so "matrix * vector" is the usual product.
class Gf2Matrix128{
public:
    static Gf2Matrix128 identity();

    const Gf2Vec128& operator[](size_t row) const{ return m_rows[row]; }
    Gf2Vec128& operator[](size_t row){ return m_rows[row]; }

    bool operator==(const Gf2Matrix128& x) const{ return m_rows == x.m_rows; }
    bool operator!=(const Gf2Matrix128& x) const{ return !(*this == x); }

    Gf2Matrix128 operator*(const Gf2Matrix128& x) const;
    Gf2Vec128 operator*(const Gf2Vec128& column) const;

    Gf2Matrix128 pow(uint64_t exponent) const;

private:
    std::array<Gf2Vec128, 128> m_rows;
};


struct Gf2SolveResult{
    //  False means the system has no solution at all. This normally indicates
    //  corrupt observations rather than a bug in the caller.
    bool consistent = false;

    //  One particular solution. Only meaningful if "consistent".
    Gf2Vec128 solution;

    //  Zero means the solution is unique. Anything larger means the
    //  observations under-determine the state and more are needed.
    size_t null_space_dimension = 0;

    //  Basis of the null space. Adding any XOR-combination of these to
    //  "solution" gives another valid solution.
    std::vector<Gf2Vec128> null_space_basis;
};

//  Solve "equations * x = rhs" over GF(2), where each entry of "equations" is one
//  row of coefficients and the matching entry of "rhs" is that row's constant.
//  The system may have any number of rows; more than 128 is normal and desirable.
Gf2SolveResult gf2_solve_128(
    const std::vector<Gf2Vec128>& equations,
    const std::vector<bool>& rhs
);


}
}
#endif
