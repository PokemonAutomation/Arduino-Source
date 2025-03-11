/*  Binary Image
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonFramework_BinaryImage_H
#define PokemonAutomation_CommonFramework_BinaryImage_H

#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix.h"

namespace PokemonAutomation{

// A wrapper class of the packed binary matrix base class `Kernels::PackedBinaryMatrix_IB`.
// Those binary matrices are memory-efficient: each binary element is stored as just one bit in memory.
// They are mainly used by the Waterfill algorithm.
// See the comments of `PackedBinaryMatrix_IB` for more implementation details.
//
// This wrapper is useful in making the usage of matrix as a locally-defined class: you can use it like
// `PackedBinaryMatrix matrix;`
// Otherwise, because of
// `PackedBinaryMatrix_IB` being polymorphic, we have to write explictly
// `std::unique_ptr<Kernels::PackedBinaryMatrix_IB>` to define a matrix.
class PackedBinaryMatrix{
public:
    //  Rule of 5
    PackedBinaryMatrix(PackedBinaryMatrix&& x) : m_matrix(std::move(x.m_matrix)) {}
    void operator=(PackedBinaryMatrix&& x){ m_matrix = std::move(x.m_matrix); }
//    PackedBinaryMatrix(const PackedBinaryMatrix& x) : m_matrix(x.m_matrix->clone()) {}
//    void operator=(const PackedBinaryMatrix& x){ m_matrix = x.m_matrix->clone(); }

    //  Don't allow implicit copying.
    PackedBinaryMatrix copy() const{ return m_matrix->clone(); };

public:
    //  Construction
    PackedBinaryMatrix();
    PackedBinaryMatrix(size_t width, size_t height);

    Kernels::BinaryMatrixType type() const{ return m_matrix->type(); }

    void clear(){ m_matrix->clear(); }

    //  Zero the entire matrix.
    void set_zero(){ return m_matrix->set_zero(); }

    //  Set entire matrix to ones.
    void set_ones(){ return m_matrix->set_ones(); }

    //  Invert all bits.
    void invert(){ return m_matrix->invert(); }

    //  Bitwise with another matrix. Dimensions must be the same!
    void operator^=(const PackedBinaryMatrix& x){ *m_matrix ^= *x.m_matrix; }
    void operator|=(const PackedBinaryMatrix& x){ *m_matrix |= *x.m_matrix; }
    void operator&=(const PackedBinaryMatrix& x){ *m_matrix &= *x.m_matrix; }

    // Print entire binary matrix as 0s and 1s. Rows are ended with "\n".
    std::string dump() const{ return m_matrix->dump(); }
    // Print part of max as 0s and 1s. Rows are ended with "\n".
    std::string dump(size_t min_x, size_t min_y, size_t max_x, size_t max_y) const{ return m_matrix->dump(min_x, min_y, max_x, max_y); }

public:
    size_t width() const{ return m_matrix->width(); }
    size_t height() const{ return m_matrix->height(); }

    //  These are slow.
    bool get(size_t x, size_t y) const{ return m_matrix->get(x, y); }
    void set(size_t x, size_t y, bool set){ m_matrix->set(x, y, set); }

    PackedBinaryMatrix submatrix(size_t x, size_t y, size_t width, size_t height) const{ return m_matrix->submatrix(x, y, width, height); }

public:
    PackedBinaryMatrix(std::unique_ptr<Kernels::PackedBinaryMatrix_IB> native)
        : m_matrix(std::move(native))
    {}

    operator const Kernels::PackedBinaryMatrix_IB&() const{ return *m_matrix; }
    operator       Kernels::PackedBinaryMatrix_IB&()      { return *m_matrix; }

private:
    std::unique_ptr<Kernels::PackedBinaryMatrix_IB> m_matrix;
};





}
#endif
