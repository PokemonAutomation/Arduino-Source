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
 *  If an exception is thrown from a parent scope, it will automatically cancel
 *  all child nodes.
 *
 */

#ifndef PokemonAutomation_CancellableScope_H
#define PokemonAutomation_CancellableScope_H

#include <chrono>
#include "Pimpl.h"

namespace PokemonAutomation{


class CancellableScopeImpl;


class CancellableScope{
public:
    CancellableScope();
    CancellableScope(CancellableScope& parent);
    ~CancellableScope();

    bool stopped() const;
    void check_stopped();   //  Throws if this scope has been cancelled.

    void cancel();

    void wait_for(std::chrono::milliseconds duration);
    void wait_until(std::chrono::system_clock::time_point stop);

private:
    Pimpl<CancellableScopeImpl> m_impl;
};



}
#endif
