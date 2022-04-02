/*  Cancellable Scope
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      A cancellable scope is a node with a tree. If "cancel()" is called on
 *  a scope, only the scope and all child scopes will be canceled. Parents are
 *  not affected.
 *
 *  This is used in nested async-cancel routines.
 *
 *  If the user stops the program, "cancel()" is called on the root node which
 *  will propagate down the entire tree.
 *
 *  If a subroutine cancels due to an inference trigger, it ends just that
 *  scope and passes control up to the parent.
 *
 *  The lifetime of a parent must entirely enclose that of the children and
 *  attached cancellables. This must hold even when an exception is thrown.
 *
 */

#ifndef PokemonAutomation_CancellableScope_H
#define PokemonAutomation_CancellableScope_H

#include <chrono>
#include <atomic>
#include "Pimpl.h"

namespace PokemonAutomation{


class CancellableScope;

class Cancellable{
public:
    Cancellable()
        : m_cancelled(false)
    {}
    Cancellable(CancellableScope& scope);
    virtual ~Cancellable(){
        detach();
    }

    //  Returns true if it was already cancelled.
    virtual bool cancel() noexcept{
        if (cancelled()){
            return true;
        }
        return m_cancelled.exchange(true);
    }

    CancellableScope* scope(){ return m_scope; }

    bool cancelled() const{
        return m_cancelled.load(std::memory_order_acquire);
    }
    void throw_if_parent_cancelled();

protected:
    //  If you inherit from this class, you may need to manually call this in the
    //  destructor. "cancel()" can be called asynchronously at any time by the
    //  parent scope. To prevent it from being called in the middle of destruction
    //  you must detach it from the parent at the start of the destructor.
    void detach() noexcept;

private:
    CancellableScope* m_scope = nullptr;
    std::atomic<bool> m_cancelled;
};


struct CancellableScopeData;
class CancellableScope final : public Cancellable{
public:
    CancellableScope();
    CancellableScope(CancellableScope& parent);
    virtual ~CancellableScope() override;

    void throw_if_cancelled();  //  Throws "OperationCanceledException" if this scope has been cancelled.

    virtual bool cancel() noexcept override;

    void wait_for(std::chrono::milliseconds duration);
    void wait_until(std::chrono::system_clock::time_point stop);

private:
    friend class Cancellable;
    void operator+=(Cancellable& cancellable);
    void operator-=(Cancellable& cancellable);

private:
    Pimpl<CancellableScopeData> m_impl;
};



inline Cancellable::Cancellable(CancellableScope& scope)
    : m_scope(&scope)
    , m_cancelled(false)
{
    scope += *this;
}
inline void Cancellable::throw_if_parent_cancelled(){
    if (m_scope){
        m_scope->throw_if_cancelled();
    }
}
inline void Cancellable::detach() noexcept{
    if (m_scope){
        *m_scope -= *this;
    }
}







}
#endif
