/*  Binary Matrix
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This class represents a memory-efficient binary matrix where each bit
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

//  Folder structure:
//  - Kernels_BinaryMatrix.h:
//    Defines BinaryMatrixType, PackedBinaryMatrix_IB, SparseBinaryMatrix_IB and declares the basic functions to get/create them.
//    PackedBinaryMatrix_IB and SparseBinaryMatrix_IB are abstract base classes (ABCs) with no function definitions.
//
//  - Kernels_BinaryMatrix.cpp:
//    Defines the basic functions to create BinaryMatrixType and the two matrices.
//    Note the matrix creation functions are large switch-case code based on CPU architecture. In each case, it calls the
//    functinos like make_PackedBinaryMatrix_<row>>x<col>_<cpu_arch>() which do the actual work. Those architecture-specific
//    functions are defined in Kernels_BinaryMatrix_Core_<cpu_arch>.cpp.
//
//  - Kernels_BinaryMatrix_t.h:
//    Defines PackedBinaryMatrix_t and SparseBinaryMatrix_t: template classes that implement the ABCs defined in
//    Kernels_BinaryMatrix.h.
//    The template parameter is Tile, an architecture-specific class that handles SIMD operations on a tile of the matrix content.
//    The template classes are just wrappers of PackedBinaryMatrixCore<Tile> and SparseBinaryMatrixCore<Tile> defined in
//    Kernels_PackedBinaryMatrixCore.h and Kernels_SparseBinaryMatrixCore.h, which host the actual implementation of matrix
//    functions.
//
//  - Kernels_PackedBinaryMatrixCore.h:
//    Defines PackedBinaryMatrixCore<Tile> wrapped by PackedBinaryMatrix_t. The native code in PackedBinaryMatrixCore is
//    architecture-agnostic. It relies on the template parameter Tile for architecture-specific code.
//    The Tiles are difined in Kernels_BinaryMatrix_Tile_<row>x<col>_<cpu_arch>.h.
//
//  - Kernels_PackedBinaryMatrixCore.tpp:
//    Defines the functions in PackedBinaryMatrixCore<Tile>.
//
//  - Kernels_SparseBinaryMatrixCore.h, Kernels_SparseBinaryMatrixCore.tpp:
//    Same to Kernels_PackedBinaryMatrixCore.h and Kernels_PackedBinaryMatrixCore.tpp but for SparseBinaryMatrixCore<Tile>.
//
//  - Kernels_BinaryMatrix_Arch_<row>x<col>_<cpu_arch>.h:
//    Defines aliases for PackedBinaryMatrix_t<Tile> and PackedBinaryMatrixCore<Tile> so they are easier to use.
//    e.g. PackedBinaryMatrix_t<BinaryTile_<row>x<col>_<cpu_arch>> is defined as PackedBinaryMatrix_<row>x<col>_<cpu_arch>.
//
//  - Kernels_BinaryMatrix_Core_<cpu_arch>.cpp:
//    Defines the boiler-plate functions of creating architecture-specific matrices, needed by Kernels_BinaryMatrix.cpp.
//    Those functions just call the constructors of the architecture-specific template classes e.g.
//    PackedBinaryMatrix_<row>x<col>_<cpu_arch>.
//
//  - Kernels_BinaryMatrixTile_<row>x<col>_<cpu_arch>.h:
//    The implementation of the architecture-specific tile class BinaryTile_<row>x<col>_<cpu_arch> used as template parameter
//    of Kernels_Packed/SparseBinaryMatrixCore<> and Packed/SparseBinaryMatrix_t<>

#ifndef PokemonAutomation_Kernels_PackedBinaryMatrix_H
#define PokemonAutomation_Kernels_PackedBinaryMatrix_H

#include <memory>
#include <string>

namespace PokemonAutomation{
namespace Kernels{

// The binary matrix type, which is just what tile shape the matrix uses.
// The shape of the tile is associated with what CPU feature to use.
// For example, for AVX2 feature, the optimized shape is 64x16.
enum class BinaryMatrixType{
    // Default impl. with no SIMD intrinsics, tile size 64-bit width x 4-bit height
    i64x4_Default,
    // Default impl. with no SIMD intrinsics, tile size 64-bit width x 8-bit height
    i64x8_Default,
    // Use Intel SSE4.2, tile size 64-bit width x 8-bit height
    i64x8_x64_SSE42,
    // Use Intel AVX 2, tile size 64-bit width x 16-bit height
    i64x16_x64_AVX2,
    // Use Intel AVX 512, tile size 64-bit width x 64-bit height
    i64x64_x64_AVX512,
    // Use Intel AVX 512, tile size 64-bit width x 32-bit height
    i64x32_x64_AVX512,
    // Use Arm NEON, tile size 64-bit width x 8-bit height
    arm64x8_x64_NEON,
};

// Get the current active binary matrix type that will be used or is being used
// by waterfill functions and others.
BinaryMatrixType get_BinaryMatrixType();

// Abstract class for all implmentations of packed binary matrices.
// Those binary matrices are memory-efficient: each binary element is stored as just one bit in memory.
// The representation uses "tiles". So instead of having each row contiguous in memory, the space is
// broken up into "tiles" that provide some vertical spatial locality.
// This class is mainly used by the Waterfill algorithm which needs vertical locality in memory.
//
// Different implmenetations differ by using different shapes/sizes of tiles.
// To avoid propagating header info of all the implementations and tiles to the code that calls the
// matrices, we use this abstract base class to have minimum exposure to the caller code.
// See "Kernels_BinaryMatrix_t.h" for its derived class, `PackedBinaryMatrix_t`, which is a template
// (with the tile class as the template parameter) and implements some boilerplate matrix-related logic.
// The tile-specific logic is written in each final derived classes, further derived from `PackedBinaryMatrix_t`.
//
// Suffix _IB stands for "internal base (class)".
// Internal means inside the kernel namespace and Kernel/BinaryMatrix folder.
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

    // Print entire binary matrix as 0s and 1s. Rows are ended with "\n".
    virtual std::string dump() const = 0;
    // Print part of max as 0s and 1s. Rows are ended with "\n".
    virtual std::string dump(size_t min_x, size_t min_y, size_t max_x, size_t max_y) const = 0;
    // Print all the underlying tiles that form this binary matrix. The result is a matrix that may be larger
    // than the original matrix.
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
