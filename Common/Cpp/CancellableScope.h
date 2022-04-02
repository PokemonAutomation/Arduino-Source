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
#include "Pimpl.h"

namespace PokemonAutomation{


class CancellableScope;

class Cancellable{
public:
    Cancellable() = default;
    Cancellable(CancellableScope& scope);
    virtual ~Cancellable(){
        detach();
    }
    virtual void cancel() noexcept = 0;

    void check_parent_cancelled();

protected:
    //  If you inherit from this class, you may need to manually call this in the
    //  destructor. "cancel()" can be called asynchronously at any time by the
    //  parent scope. To prevent it from being called in the middle of destruction
    //  you must detach it from the parent at the start of the destructor.
    void detach() noexcept;

private:
    CancellableScope* m_scope = nullptr;
};


class CancellableScopeImpl;
class CancellableScope final : public Cancellable{
public:
    CancellableScope();
    CancellableScope(CancellableScope& parent);
    virtual ~CancellableScope() override;

    bool cancelled() const;
    void check_cancelled();     //  Throws "OperationCanceledException" if this scope has been cancelled.


    virtual void cancel() noexcept override;

    void wait_for(std::chrono::milliseconds duration);
    void wait_until(std::chrono::system_clock::time_point stop);

private:
    friend class Cancellable;
    void operator+=(Cancellable& cancellable);
    void operator-=(Cancellable& cancellable);

private:
    Pimpl<CancellableScopeImpl> m_impl;
};



inline Cancellable::Cancellable(CancellableScope& scope)
    : m_scope(&scope)
{
    scope += *this;
}
inline void Cancellable::check_parent_cancelled(){
    if (m_scope){
        m_scope->check_cancelled();
    }
}
inline void Cancellable::detach() noexcept{
    if (m_scope){
        *m_scope -= *this;
    }
}







}
#endif
