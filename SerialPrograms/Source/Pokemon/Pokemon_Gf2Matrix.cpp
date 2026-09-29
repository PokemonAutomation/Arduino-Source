/*  GF(2) Linear Algebra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <bit>
#include "Common/Cpp/Exceptions.h"
#include "Pokemon_Gf2Matrix.h"

namespace PokemonAutomation{
namespace Pokemon{



bool Gf2Vec128::get(size_t index) const{
    return index < 64
        ? ((high >> (63 - index)) & 1) != 0
        : ((low >> (127 - index)) & 1) != 0;
}
void Gf2Vec128::set(size_t index, bool value){
    uint64_t& word = index < 64 ? high : low;
    uint64_t mask = (uint64_t)1 << (index < 64 ? 63 - index : 127 - index);
    if (value){
        word |= mask;
    }else{
        word &= ~mask;
    }
}
bool Gf2Vec128::dot(const Gf2Vec128& x) const{
    int bits = std::popcount(high & x.high) + std::popcount(low & x.low);
    return (bits & 1) != 0;
}



Gf2Matrix128 Gf2Matrix128::identity(){
    Gf2Matrix128 ret;
    for (size_t c = 0; c < 128; c++){
        ret.m_rows[c].set(c, true);
    }
    return ret;
}

//  row i of the product is the XOR of every row k in x that has x[k][i] == 1.
Gf2Matrix128 Gf2Matrix128::operator*(const Gf2Matrix128& x) const{
    Gf2Matrix128 ret;
    for (size_t i = 0; i < 128; i++){
        Gf2Vec128 accumulator;
        uint64_t bits = m_rows[i].high;
        while (bits != 0){
            //  index = 63 - position.
            accumulator ^= x.m_rows[63 - (size_t)std::countr_zero(bits)];
            bits &= bits - 1;
        }
        bits = m_rows[i].low;
        while (bits != 0){
            //  index = 127 - position.
            accumulator ^= x.m_rows[127 - (size_t)std::countr_zero(bits)];
            bits &= bits - 1;
        }
        ret.m_rows[i] = accumulator;
    }
    return ret;
}
Gf2Vec128 Gf2Matrix128::operator*(const Gf2Vec128& column) const{
    Gf2Vec128 ret;
    for (size_t c = 0; c < 128; c++){
        ret.set(c, m_rows[c].dot(column));
    }
    return ret;
}

Gf2Matrix128 Gf2Matrix128::pow(uint64_t exponent) const{
    Gf2Matrix128 ret = identity();
    Gf2Matrix128 base = *this;
    while (exponent != 0){
        if ((exponent & 1) != 0){
            ret = ret * base;
        }
        exponent >>= 1;
        if (exponent != 0){
            base = base * base;
        }
    }
    return ret;
}

Gf2SolveResult gf2_solve_128(
    const std::vector<Gf2Vec128>& equations,
    const std::vector<bool>& rhs
){
    if (equations.size() != rhs.size()){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "gf2_solve_128(): Coefficient and constant counts do not match."
        );
    }

    //  Augmented system. We reduce to row echelon form, then back-substitute.
    std::vector<Gf2Vec128> rows = equations;
    std::vector<bool> constants = rhs;
    const size_t height = rows.size();

    //  pivot_row[c] is the row that owns column "c" as its pivot, or NO_PIVOT.
    const size_t NO_PIVOT = (size_t)0 - 1;
    std::array<size_t, 128> pivot_row;
    pivot_row.fill(NO_PIVOT);

    size_t next_row = 0;
    for (size_t column = 0; column < 128 && next_row < height; column++){
        size_t pivot = NO_PIVOT;
        for (size_t row = next_row; row < height; row++){
            if (rows[row].get(column)){
                pivot = row;
                break;
            }
        }
        if (pivot == NO_PIVOT){
            continue;
        }

        std::swap(rows[next_row], rows[pivot]);
        {
            //  std::vector<bool> has no swappable references.
            bool tmp = constants[next_row];
            constants[next_row] = constants[pivot];
            constants[pivot] = tmp;
        }

        for (size_t row = 0; row < height; row++){
            if (row != next_row && rows[row].get(column)){
                rows[row] ^= rows[next_row];
                constants[row] = constants[row] != constants[next_row];
            }
        }

        pivot_row[column] = next_row;
        next_row++;
    }

    Gf2SolveResult result;

    //  Any all-zero row with a nonzero constant makes the system unsolvable.
    for (size_t row = 0; row < height; row++){
        if (rows[row].is_zero() && constants[row]){
            return result;
        }
    }
    result.consistent = true;

    //  Free variables are set to zero, so each pivot variable is just its constant.
    for (size_t column = 0; column < 128; column++){
        if (pivot_row[column] != NO_PIVOT){
            result.solution.set(column, constants[pivot_row[column]]);
        }
    }

    //  One null space basis vector per free column.
    for (size_t column = 0; column < 128; column++){
        if (pivot_row[column] != NO_PIVOT){
            continue;
        }
        Gf2Vec128 basis;
        basis.set(column, true);
        for (size_t pivot_column = 0; pivot_column < 128; pivot_column++){
            size_t row = pivot_row[pivot_column];
            if (row != NO_PIVOT && rows[row].get(column)){
                basis.set(pivot_column, true);
            }
        }
        result.null_space_basis.emplace_back(basis);
    }
    result.null_space_dimension = result.null_space_basis.size();

    return result;
}




}
}
