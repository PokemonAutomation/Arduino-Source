/*  Pimpl
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pimpl_H
#define PokemonAutomation_Pimpl_H

namespace PokemonAutomation{


template <typename Type>
class Pimpl{
public:
    ~Pimpl();
    Pimpl(Pimpl&& x);
    void operator=(Pimpl&& x);
    Pimpl(const Pimpl& x);
    void operator=(const Pimpl& x);


public:
    template <class... Args>
    Pimpl(Args&&... args);

    operator const Type&() const    { return *m_ptr; }
    operator       Type&()          { return *m_ptr; }
    const Type* operator->() const  { return m_ptr; }
          Type* operator->()        { return m_ptr; }

    const Type* get() const         { return m_ptr; }
          Type* get()               { return m_ptr; }


private:
    Type* m_ptr;
};



template <typename Type>
Pimpl<Type>::Pimpl(Pimpl&& x)
    : m_ptr(x.m_ptr)
{
    x.m_ptr = nullptr;
}
template <typename Type>
void Pimpl<Type>::operator=(Pimpl&& x){
    if (this == &x){
        return;
    }
    m_ptr = x.m_ptr;
    x.m_ptr = nullptr;
}





}
#endif
