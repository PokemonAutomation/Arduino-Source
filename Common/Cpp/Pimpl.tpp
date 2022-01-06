/*  Pimpl
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pimpl_TPP
#define PokemonAutomation_Pimpl_TPP

#include <new>
#include <utility>
#include "Pimpl.h"

namespace PokemonAutomation{



template <typename Type>
Pimpl<Type>::~Pimpl(){
    delete m_ptr;
}

template <typename Type>
Pimpl<Type>::Pimpl(const Pimpl& x)
    : m_ptr(new Type(*x.m_ptr))
{}
template <typename Type>
void Pimpl<Type>::operator=(const Pimpl& x){
    if (this == &x){
        return;
    }
    Type* copy = new Type(*x.m_ptr);
    Type* ptr = m_ptr;
    m_ptr = copy;
    delete ptr;
}

template <typename Type>
template <class... Args>
Pimpl<Type>::Pimpl(Args&&... args)
    : m_ptr(new Type(std::forward<Args>(args)...))
{}




}
#endif
