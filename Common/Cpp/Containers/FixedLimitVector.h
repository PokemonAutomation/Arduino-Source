/*  Fixed Limit Vector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      A non-copyable, non-movable vector whose buffer cannot be resized.
 *  It supports emplace_back() and pop_back(). All elements are unchanged
 *  through either operation unless it's the one being popped by pop_back().
 *
 *  There are no requirements of the elements. They need not be copyable nor
 *  movable since they will stay in place.
 *
 *
 *      This file will compile with an imcomplete type for headers. But
 *  you need to include "FixedLimitVector.tpp" to really use it.
 *
 */

#ifndef PokemonAutomation_FixedLimitVector_H
#define PokemonAutomation_FixedLimitVector_H

#include <stddef.h>

namespace PokemonAutomation{


template <typename Object>
class FixedLimitVector{
public:
    ~FixedLimitVector();
    FixedLimitVector(const FixedLimitVector&) = delete;
    void operator=(const FixedLimitVector&) = delete;
    FixedLimitVector(FixedLimitVector&& x);
    void operator=(FixedLimitVector&& x);

public:
    FixedLimitVector();
    FixedLimitVector(size_t capacity);

    void reset();
    void reset(size_t capacity);

public:
    size_t size() const{ return m_size; }
    size_t capacity() const{ return m_capacity; }

    const Object& operator[](size_t index) const{ return m_data[index]; }
          Object& operator[](size_t index)      { return m_data[index]; }
    const Object& back() const{ return m_data[m_size - 1]; }
          Object& back()      { return m_data[m_size - 1]; }

    template <class... Args>
    bool emplace_back(Args&&... args);
    void pop_back();

    const Object* begin() const{ return m_data; }
          Object* begin()      { return m_data; };
    const Object* end() const{ return m_data + m_size; }
          Object* end()      { return m_data + m_size; }

    const Object* data() const{ return m_data; }
          Object* data()      { return m_data; };

private:
    Object* m_data;
    size_t m_size;
    size_t m_capacity;
};





//  Implementations

template <typename Object>
FixedLimitVector<Object>::FixedLimitVector(FixedLimitVector&& x)
    : m_data(x.m_data)
    , m_size(x.m_size)
    , m_capacity(x.m_capacity)
{
    x.m_data = nullptr;
    x.m_size = 0;
    x.m_capacity = 0;
}
template <typename Object>
void FixedLimitVector<Object>::operator=(FixedLimitVector&& x){
    reset();
    m_data = x.m_data;
    m_size = x.m_size;
    m_capacity = x.m_capacity;
    x.m_data = nullptr;
    x.m_size = 0;
    x.m_capacity = 0;
}

template <typename Object>
FixedLimitVector<Object>::FixedLimitVector()
    : m_data(nullptr)
    , m_size(0)
    , m_capacity(0)
{}




}
#endif
