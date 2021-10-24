/*  Aligned Vector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AlignedVector_H
#define PokemonAutomation_AlignedVector_H

#include <new>
#include <type_traits>
#include <utility>
#include <immintrin.h>
#include "Common/Compiler.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


template <typename Object>
class AlignedVector{
public:
    ~AlignedVector();
    AlignedVector(AlignedVector&& x);
    void operator=(AlignedVector&& x);
    AlignedVector(const AlignedVector&) = delete;
    void operator=(const AlignedVector&) = delete;

public:
    AlignedVector();
    AlignedVector(size_t items);

public:
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

    void clear();

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
AlignedVector<Object>::~AlignedVector(){
    clear();
    _mm_free(m_ptr);
    m_capacity = 0;
}
template <typename Object>
AlignedVector<Object>::AlignedVector(AlignedVector&& x)
    : m_ptr(x.m_ptr)
    , m_size(x.m_size)
    , m_capacity(x.m_capacity)
{
    x.m_ptr = nullptr;
    x.m_size = 0;
    x.m_capacity = 0;
}
template <typename Object>
void AlignedVector<Object>::operator=(AlignedVector&& x){
    m_ptr = x.m_ptr;
    m_size = x.m_size;
    m_capacity = x.m_capacity;
    x.m_ptr = nullptr;
    x.m_size = 0;
    x.m_capacity = 0;
}

template <typename Object>
AlignedVector<Object>::AlignedVector()
    : m_ptr(nullptr)
    , m_size(0)
    , m_capacity(0)
{}
template <typename Object>
AlignedVector<Object>::AlignedVector(size_t items){
    m_ptr = (Object*)_mm_malloc(items * sizeof(Object), 64);
    if (m_ptr == nullptr){
        throw std::bad_alloc();
    }
    m_capacity = items;

    if (std::is_trivially_constructible<Object>::value){
        m_size = items;
        return;
    }

    m_size = 0;
    try{
        for (size_t c = 0; c < items; c++){
            new (m_ptr + m_size) Object;
            m_size++;
        }
    }catch (...){
        clear();
        _mm_free(m_ptr);
    }
}

template <typename Object>
template <class... Args>
void AlignedVector<Object>::emplace_back(Args&&... args){
    if (m_size >= m_capacity){
        expand();
    }
    new (m_ptr + m_size) Object(std::forward<Args>(args)...);
    m_size++;
}
template <typename Object>
void AlignedVector<Object>::pop_back(){
    m_ptr[--m_size].~Object();
}
template <typename Object>
void AlignedVector<Object>::clear(){
    while (m_size > 0){
        pop_back();
    }
}

template <typename Object>
PA_NO_INLINE void AlignedVector<Object>::expand(){
    size_t size = m_capacity == 0 ? 1 : m_capacity * 2;
    Object* ptr = (Object*)_mm_malloc(size * sizeof(Object), 64);
    if (ptr == nullptr){
        throw std::bad_alloc();
    }
    for (size_t c = 0; c < m_size; c++){
        new (ptr + c) Object(std::move(m_ptr[c]));
        m_ptr[c].~Object();
    }
    _mm_free(m_ptr);
    m_ptr = ptr;
    m_capacity = size;
}

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
