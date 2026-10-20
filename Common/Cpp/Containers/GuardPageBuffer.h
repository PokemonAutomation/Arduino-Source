/*  Guard Page Buffer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  An array that ends exactly where a no-access guard page begins. Reading or
 *  writing even one byte past the end faults right away, instead of silently
 *  touching whatever memory happens to be next to it.
 *
 *  This is meant for unit tests of SIMD kernels. A tail that loads or stores a
 *  full vector past the end of the data then crashes on every run, not just on
 *  the rare run where the heap happens to end there.
 *
 *  Because the end is pinned to a page boundary, the start is only aligned to
 *  alignof(Object) unless you ask for more. With a larger alignment, the start
 *  is rounded down to it, which leaves a gap of up to alignment - 1 bytes
 *  between the end of the data and the guard page when the size is not a
 *  multiple of the alignment. Reads and writes into that gap are not caught.
 *
 *  Every buffer maps at least two pages, so this is not for production code.
 *
 */

#ifndef PokemonAutomation_GuardPageBuffer_H
#define PokemonAutomation_GuardPageBuffer_H

#include <stddef.h>
#include <type_traits>

namespace PokemonAutomation{


//  Maps enough pages for "bytes" bytes, followed by one no-access page.
//  data() points to the first byte and is a multiple of "alignment" (a power
//  of two). The data ends right before the guard page when "bytes" is a
//  multiple of "alignment". The memory starts out zeroed. Throws
//  std::bad_alloc on failure.
class GuardPageMemory{
public:
    ~GuardPageMemory();
    GuardPageMemory(GuardPageMemory&& x) noexcept;
    void operator=(GuardPageMemory&& x) noexcept;
    GuardPageMemory(const GuardPageMemory&) = delete;
    void operator=(const GuardPageMemory&) = delete;

public:
    GuardPageMemory(size_t bytes, size_t alignment = 1);

    void* data() const{ return m_data; }

private:
    void release() noexcept;

private:
    void* m_base;
    size_t m_mapped_bytes;
    void* m_data;
};



template <typename Object>
class GuardPageBuffer{
    static_assert(
        std::is_trivially_default_constructible_v<Object> &&
        std::is_trivially_destructible_v<Object>,
        "GuardPageBuffer only holds trivial types. The elements start as zero bytes."
    );

public:
    GuardPageBuffer(size_t size, size_t alignment = alignof(Object))
        : m_memory(size * sizeof(Object), alignment)
        , m_size(size)
    {}

    size_t size() const{ return m_size; }

    const Object& operator[](size_t index) const{ return data()[index]; }
          Object& operator[](size_t index)      { return data()[index]; }

    const Object* data() const{ return (const Object*)m_memory.data(); }
          Object* data()      { return (Object*)m_memory.data(); }

private:
    GuardPageMemory m_memory;
    size_t m_size;
};



}
#endif
