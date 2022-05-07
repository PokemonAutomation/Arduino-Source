/*  Binary Matrix
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This class represents a memory-efficient binary matrix where each bit
 *  is stored as just one bit in memory.
 *
 *  The representation uses "tiles". So instead of having each row contiguous
 *  in memory, the space is broken up into "tiles" that provide some vertical
 *  spatial locality.
 *
 *  This class is mainly used by the Waterfill algorithm which needs vertical
 *  locality in memory.
 *
 */

#ifndef PokemonAutomation_Kernels_PackedBinaryMatrix_H
#define PokemonAutomation_Kernels_PackedBinaryMatrix_H

#include <memory>
#include <string>

namespace PokemonAutomation{
namespace Kernels{


enum class BinaryMatrixType{
    i64x4_Default,
    i64x8_SSE42,
    i64x16_AVX2,
    i64x64_AVX512,
    i64x32_AVX512,
};

BinaryMatrixType get_BinaryMatrixType();


class PackedBinaryMatrix_IB{
public:
    virtual ~PackedBinaryMatrix_IB() = default;

public:
    virtual BinaryMatrixType type() const = 0;
    virtual std::unique_ptr<PackedBinaryMatrix_IB> clone() const = 0;

    virtual void clear() = 0;

    virtual void set_zero() = 0;    //  Zero the entire matrix.
    virtual void set_ones() = 0;    //  Set entire matrix to ones.
    virtual void invert() = 0;      //  Invert all bits.

    //  Matrix must have same dimensions.
    virtual void operator^=(const PackedBinaryMatrix_IB& x) = 0;
    virtual void operator|=(const PackedBinaryMatrix_IB& x) = 0;
    virtual void operator&=(const PackedBinaryMatrix_IB& x) = 0;

    virtual std::string dump() const = 0;
    virtual std::string dump(size_t min_x, size_t min_y, size_t max_x, size_t max_y) const = 0;
    virtual std::string dump_tiles() const = 0;

public:
    virtual size_t width() const = 0;
    virtual size_t height() const = 0;

    //  These are slow.
    virtual bool get(size_t x, size_t y) const = 0;
    virtual void set(size_t x, size_t y, bool set) = 0;

    virtual std::unique_ptr<PackedBinaryMatrix_IB> submatrix(size_t x, size_t y, size_t width, size_t height) const = 0;
};
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix(BinaryMatrixType type);
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix(BinaryMatrixType type, size_t width, size_t height);



class SparseBinaryMatrix_IB{
public:
    virtual ~SparseBinaryMatrix_IB() = default;

public:
    virtual BinaryMatrixType type() const = 0;
    virtual std::unique_ptr<SparseBinaryMatrix_IB> clone() const = 0;

    virtual void clear() = 0;

    virtual void operator^=(const SparseBinaryMatrix_IB& x) = 0;
    virtual void operator|=(const SparseBinaryMatrix_IB& x) = 0;
    virtual void operator&=(const SparseBinaryMatrix_IB& x) = 0;

public:
    virtual size_t width() const = 0;
    virtual size_t height() const = 0;

    //  These are slow.
    virtual bool get(size_t x, size_t y) const = 0;
    virtual void set(size_t x, size_t y, bool set) = 0;

    virtual std::unique_ptr<PackedBinaryMatrix_IB> submatrix(size_t x, size_t y, size_t width, size_t height) const = 0;

    virtual std::string dump() const = 0;
    virtual std::string dump(size_t min_x, size_t min_y, size_t max_x, size_t max_y) const = 0;
};
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix(BinaryMatrixType type);
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix(BinaryMatrixType type, size_t width, size_t height);








}
}
#endif
