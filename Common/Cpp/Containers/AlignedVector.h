/*  Aligned Vector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AlignedVector_H
#define PokemonAutomation_AlignedVector_H

#include <stddef.h>

namespace PokemonAutomation{


template <typename Object>
class AlignedVector{
public:
    ~AlignedVector();
    AlignedVector(AlignedVector&& x) noexcept;
    void operator=(AlignedVector&& x) noexcept;
    AlignedVector(const AlignedVector& x);
    void operator=(const AlignedVector& x);

public:
    AlignedVector();
    AlignedVector(size_t items);

    void clear() noexcept;

public:
    size_t empty() const{ return m_size == 0; }
    size_t size() const{ return m_size; }
    size_t capacity() const{ return m_capacity; }

    const Object& operator[](size_t index) const{ return m_ptr[index]; }
          Object& operator[](size_t index)      { return m_ptr[index]; }
    const Object& back() const{ return m_ptr[m_size - 1]; }
          Object& back()      { return m_ptr[m_size - 1]; }

    const Object* data() const{ return m_ptr; }
          Object* data()      { return m_ptr; }

    template <class... Args>
    void emplace_back(Args&&... args);
    void pop_back();

    const Object* begin() const;
    Object* begin();
    const Object* end() const;
    Object* end();

private:
    void expand();


private:
    Object* m_ptr;
    size_t m_size;
    size_t m_capacity;
};





template <typename Object>
AlignedVector<Object>::AlignedVector()
    : m_ptr(nullptr)
    , m_size(0)
    , m_capacity(0)
{}

template <typename Object>
const Object* AlignedVector<Object>::begin() const{
    return m_ptr;
}
template <typename Object>
Object* AlignedVector<Object>::begin(){
    return m_ptr;
}
template <typename Object>
const Object* AlignedVector<Object>::end() const{
    return m_ptr + m_size;
}
template <typename Object>
Object* AlignedVector<Object>::end(){
    return m_ptr + m_size;
}




}
#endif
