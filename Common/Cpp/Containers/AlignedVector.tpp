/*  Aligned Vector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AlignedVector_TPP
#define PokemonAutomation_AlignedVector_TPP

#include <string.h>
#include <new>
#include <type_traits>
#include <utility>
#include <algorithm>
#include "Common/Compiler.h"
#include "AlignedMalloc.h"
#include "AlignedVector.h"

namespace PokemonAutomation{



template <typename Object>
AlignedVector<Object>::~AlignedVector(){
    clear();
    aligned_free(m_ptr);
    m_capacity = 0;
}
template <typename Object>
AlignedVector<Object>::AlignedVector(AlignedVector&& x) noexcept
    : m_ptr(x.m_ptr)
    , m_size(x.m_size)
    , m_capacity(x.m_capacity)
{
    x.m_ptr = nullptr;
    x.m_size = 0;
    x.m_capacity = 0;
}
template <typename Object>
void AlignedVector<Object>::operator=(AlignedVector&& x) noexcept{
    clear();
    aligned_free(m_ptr);
    m_ptr = x.m_ptr;
    m_size = x.m_size;
    m_capacity = x.m_capacity;
    x.m_ptr = nullptr;
    x.m_size = 0;
    x.m_capacity = 0;
}
template <typename Object>
AlignedVector<Object>::AlignedVector(const AlignedVector& x)
    : m_capacity(x.m_capacity)
{
    //  Shrink to fit.
    while (m_capacity > 0){
        size_t half = m_capacity / 2;
        if (half >= x.m_size){
            m_capacity = half;
        } else{
            break;
        }
    }
    m_ptr = (Object*)aligned_malloc(
        m_capacity * sizeof(Object),
        std::max<size_t>(alignof(Object), 64)
    );
    if (m_ptr == nullptr){
        throw std::bad_alloc();
    }

    if constexpr (std::is_trivially_copyable<Object>::value){
        memcpy(m_ptr, x.m_ptr, x.m_size * sizeof(Object));
        m_size = x.m_size;
        return;
    }

    m_size = 0;
    try{
        for (size_t c = 0; c < x.m_size; c++){
            new (m_ptr + m_size) Object(x[c]);
            m_size++;
        }
    }catch (...){
        clear();
        aligned_free(m_ptr);
        throw;
    }
}
template <typename Object>
void AlignedVector<Object>::operator=(const AlignedVector& x){
    if (this == &x){
        return;
    }
    AlignedVector tmp(x);
    *this = std::move(tmp);
}



template <typename Object>
AlignedVector<Object>::AlignedVector(size_t items){
    m_ptr = (Object*)aligned_malloc(items * sizeof(Object), PA_ALIGNMENT);
    if (m_ptr == nullptr){
        throw std::bad_alloc();
    }
    m_capacity = items;

    if constexpr (std::is_trivially_constructible<Object>::value){
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
        aligned_free(m_ptr);
        throw;
    }
}

template <typename Object>
void AlignedVector<Object>::clear() noexcept{
    if constexpr (std::is_trivially_constructible<Object>::value){
        m_size = 0;
    }else{
        while (m_size > 0){
            pop_back();
        }
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
PA_NO_INLINE void AlignedVector<Object>::expand(){
    size_t size = m_capacity == 0 ? 1 : m_capacity * 2;
    Object* ptr = (Object*)aligned_malloc(size * sizeof(Object), PA_ALIGNMENT);
    if (ptr == nullptr){
        throw std::bad_alloc();
    }
    if constexpr (std::is_trivially_copyable<Object>::value){
        memcpy(ptr, m_ptr, m_size * sizeof(Object));
    }else{
        for (size_t c = 0; c < m_size; c++){
            new (ptr + c) Object(std::move(m_ptr[c]));
            m_ptr[c].~Object();
        }
    }
    aligned_free(m_ptr);
    m_ptr = ptr;
    m_capacity = size;
}







}
#endif
