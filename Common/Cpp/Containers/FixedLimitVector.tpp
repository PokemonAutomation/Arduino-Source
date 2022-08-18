/*  Fixed Limit Vector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_FixedLimitVector_IPP
#define PokemonAutomation_FixedLimitVector_IPP

#include <new>
#include <utility>
#include "AlignedMalloc.h"
#include "FixedLimitVector.h"

namespace PokemonAutomation{


//  Rule of 5

template <typename Object>
FixedLimitVector<Object>::~FixedLimitVector(){
    while (m_size > 0){
        pop_back();
    }
    aligned_free(m_data);
}



//  Constructors

template <typename Object>
FixedLimitVector<Object>::FixedLimitVector(size_t capacity)
    : m_size(0)
    , m_capacity(capacity)
{
    m_data = (Object*)aligned_malloc(capacity * sizeof(Object), alignof(Object));
    if (m_data == nullptr){
        throw std::bad_alloc();
    }
}

template <typename Object>
void FixedLimitVector<Object>::reset(){
    while (m_size > 0){
        pop_back();
    }
    aligned_free(m_data);
    m_data = nullptr;
    m_capacity = 0;
}
template <typename Object>
void FixedLimitVector<Object>::reset(size_t capacity){
    Object* data = (Object*)aligned_malloc(capacity * sizeof(Object), alignof(Object));
    if (data == nullptr){
        throw std::bad_alloc();
    }
    while (m_size > 0){
        pop_back();
    }
    aligned_free(m_data);
    m_data = data;
    m_capacity = capacity;
}



//  Functions

template <typename Object>
template <class... Args>
bool FixedLimitVector<Object>::emplace_back(Args&&... args){
    if (m_size < m_capacity){
        new (m_data + m_size) Object(std::forward<Args>(args)...);
        m_size++;
        return true;
    }else{
        return false;
    }
}
template <typename Object>
void FixedLimitVector<Object>::pop_back(){
    m_data[--m_size].~Object();
}






}
#endif
