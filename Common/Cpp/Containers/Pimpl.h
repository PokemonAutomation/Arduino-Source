/*  Pimpl
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pimpl_H
#define PokemonAutomation_Pimpl_H

namespace PokemonAutomation{


enum PimplConstruct{
    CONSTRUCT_TOKEN
};


template <typename Type>
class Pimpl{
public:
    ~Pimpl();
    Pimpl(Pimpl&& x) noexcept;
    void operator=(Pimpl&& x) noexcept;
    Pimpl(const Pimpl& x);
    void operator=(const Pimpl& x);


public:
    Pimpl() = default;

    template <class... Args>
    Pimpl(PimplConstruct, Args&&... args);

    void clear();

    template <class... Args>
    void reset(Args&&... args);


public:
    operator bool() const{ return m_ptr != nullptr; }

    operator const Type&() const{ return *m_ptr; }
    operator       Type&()      { return *m_ptr; }

    const Type& operator*() const   { return *m_ptr; }
          Type& operator*()         { return *m_ptr; }

    const Type* operator->() const  { return m_ptr; }
          Type* operator->()        { return m_ptr; }

    const Type* get() const         { return m_ptr; }
          Type* get()               { return m_ptr; }


private:
    Type* m_ptr = nullptr;
};



template <typename Type>
Pimpl<Type>::Pimpl(Pimpl&& x) noexcept
    : m_ptr(x.m_ptr)
{
    x.m_ptr = nullptr;
}
template <typename Type>
void Pimpl<Type>::operator=(Pimpl&& x) noexcept{
    if (this == &x){
        return;
    }
    delete m_ptr;
    m_ptr = x.m_ptr;
    x.m_ptr = nullptr;
}





}
#endif
