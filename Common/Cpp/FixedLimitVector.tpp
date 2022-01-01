/*  Fixed Limit Vector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_FixedLimitVector_IPP
#define PokemonAutomation_FixedLimitVector_IPP

#include <new>
#include <utility>
#include "FixedLimitVector.h"

namespace PokemonAutomation{


//  Rule of 5

template <typename Object>
FixedLimitVector<Object>::~FixedLimitVector(){
    while (m_size > 0){
        pop_back();
    }
    delete[] reinterpret_cast<char*>(m_data);
}



//  Constructors

template <typename Object>
FixedLimitVector<Object>::FixedLimitVector(size_t capacity)
    : m_size(0)
    , m_capacity(capacity)
{
    m_data = reinterpret_cast<Object*>(new char[capacity * sizeof(Object)]);
}

template <typename Object>
void FixedLimitVector<Object>::reset(){
    while (m_size > 0){
        pop_back();
    }
    delete[] reinterpret_cast<char*>(m_data);
    m_data = nullptr;
    m_capacity = 0;
}
template <typename Object>
void FixedLimitVector<Object>::reset(size_t capacity){
    Object* data = reinterpret_cast<Object*>(new char[capacity * sizeof(Object)]);
    while (m_size > 0){
        pop_back();
    }
    delete[] reinterpret_cast<char*>(m_data);
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
