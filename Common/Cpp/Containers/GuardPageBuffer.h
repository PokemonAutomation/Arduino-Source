/*  Guard Page Buffer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  A test-only array that crashes on any access past its end. Unit tests of
 *  SIMD kernels use it to catch reads and writes past the end of a buffer.
 *
 */

#ifndef PokemonAutomation_GuardPageBuffer_H
#define PokemonAutomation_GuardPageBuffer_H

#include <stddef.h>
#include <type_traits>

namespace PokemonAutomation{


//  The memory behind `GuardPageBuffer`, without an element type.
//
//  It maps enough whole pages from the OS for "bytes" bytes (plus the room to
//  round the start down to "alignment"), and then one more page that it marks
//  as no-access: the guard page. `data()` points to the first byte of the
//  data, placed so that the data ends right before the guard page.
//
//  The memory starts as all zero bytes, and the destructor unmaps it. It can
//  be moved but not copied. A moved-from object owns no memory.
//
//  Throws std::bad_alloc if the OS cannot map or protect the pages, and
//  InternalProgramError if "alignment" is not a power of two.
class GuardPageMemory{
public:
    ~GuardPageMemory();
    GuardPageMemory(GuardPageMemory&& x) noexcept;
    void operator=(GuardPageMemory&& x) noexcept;
    GuardPageMemory(const GuardPageMemory&) = delete;
    void operator=(const GuardPageMemory&) = delete;

public:
    GuardPageMemory(size_t bytes, size_t alignment = 1);

    //  First byte of the data. The data ends right before the guard page
    //  when "bytes" is a multiple of "alignment".
    void* data() const{ return m_data; }

private:
    void release() noexcept;

private:
    void* m_base;           //  Start of the mapped pages, or nullptr after a move.
    size_t m_mapped_bytes;  //  Size of the mapping, guard page included.
    void* m_data;           //  What `data()` returns.
};



//  A fixed-size array for unit tests whose last element sits right before a
//  guard page: a page of memory that the program is not allowed to read or
//  write. Any access just past the end of the array crashes the test at once.
//
//  Why it exists: SIMD kernels work on whole vectors (for example, 16 floats
//  with AVX-512) and need special code for the last few elements, the "tail".
//  A tail that loads or stores a full vector touches memory past the end of
//  the buffer. With a normal heap buffer like `AlignedVector`, that memory
//  usually belongs to the heap, so nothing happens. The test only crashes on
//  the rare run where the buffer happens to end next to an unmapped page.
//  That is how an over-read in the AVX-512 `AudioChannelMixer` tail crashed
//  CI only now and then. With a GuardPageBuffer, every such run crashes.
//
//  How it works: `GuardPageMemory` maps whole pages and makes the last page
//  no-access. The array is placed so that its end touches that page.
//  Example with 4 KB pages and `GuardPageBuffer<float> buffer(18)`:
//  1. The data needs 18 * 4 = 72 bytes, so it maps 1 data page and 1 guard
//     page (8 KB in total).
//  2. `buffer.data()` points 72 bytes before the guard page.
//  3. `buffer[17]` is the last valid element. Reading `buffer[18]`, or a
//     16-float vector load at `buffer.data() + 16`, crashes.
//
//  Where it is used: the kernel unit tests use it for the input and output
//  buffers of the kernels under test, for example in
//  `Kernels_AudioChannelMixer_TestInfra.cpp`. `GuardPageImageRGB32` in
//  `Tests/GuardPageImage.h` uses it for images.
//
//  Limits:
//  - Only trivial types like float and uint32_t. The elements are not
//    constructed or destroyed, and they start as zero bytes.
//  - Only accesses after the end are caught, not accesses before the start.
//  - By default, `data()` is only aligned to alignof(Object). For a kernel
//    that needs aligned pointers, pass a larger "alignment", for example
//    PA_ALIGNMENT (64 bytes) for `fft_abs()`. The start is then rounded
//    down, which can leave a gap of up to alignment - 1 bytes before the
//    guard page. Accesses in that gap are not caught, so use sizes that are
//    a multiple of the alignment when you can.
//  - Each buffer maps at least 2 pages with a system call, so it is slower
//    than a heap buffer and only meant for tests.
//
//  Throws std::bad_alloc if the OS cannot map or protect the pages, and
//  InternalProgramError if "alignment" is not a power of two.
template <typename Object>
class GuardPageBuffer{
    static_assert(
        std::is_trivially_default_constructible_v<Object> &&
        std::is_trivially_destructible_v<Object>,
        "GuardPageBuffer only holds trivial types. The elements start as zero bytes."
    );

public:
    //  Map "size" elements, all zero bytes. "alignment" is the alignment of
    //  `data()` in bytes and must be a power of two. Example:
    //  `GuardPageBuffer<float> real(length, PA_ALIGNMENT)` gives `fft_abs()`
    //  the 64-byte aligned input that it requires.
    GuardPageBuffer(size_t size, size_t alignment = alignof(Object))
        : m_memory(size * sizeof(Object), alignment)
        , m_size(size)
    {}

    //  Number of elements, as given to the constructor.
    size_t size() const{ return m_size; }

    //  Element access without a bounds check. An index at or just past
    //  size() reads the guard page and crashes, which is what tests want.
    const Object& operator[](size_t index) const{ return data()[index]; }
          Object& operator[](size_t index)      { return data()[index]; }

    //  Pointer to the first element, to give to the kernel under test.
    const Object* data() const{ return (const Object*)m_memory.data(); }
          Object* data()      { return (Object*)m_memory.data(); }

private:
    GuardPageMemory m_memory;
    size_t m_size;
};



}
#endif
