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
 *  alignof(Object). Don't use it to test code that needs aligned pointers.
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
//  data() points to the first byte. The last byte sits right before the
//  guard page. The memory starts out zeroed. Throws std::bad_alloc on failure.
class GuardPageMemory{
public:
    ~GuardPageMemory();
    GuardPageMemory(const GuardPageMemory&) = delete;
    void operator=(const GuardPageMemory&) = delete;

public:
    GuardPageMemory(size_t bytes);

    void* data() const{ return m_data; }

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
    GuardPageBuffer(size_t size)
        : m_memory(size * sizeof(Object))
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
