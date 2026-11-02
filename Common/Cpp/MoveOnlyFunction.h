/*  Move-Only Function
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  A callable wrapper that can be moved but not copied.
 *
 *  Prefer this over std::function for a callback that has one owner.
 *  Copying a std::function copies its callable, so a stateful lambda
 *  silently splits into two independent states. And std::function cannot
 *  hold a lambda that captures a move-only object (e.g. std::unique_ptr).
 *
 *  This is std::move_only_function (C++23) where the standard library has
 *  it. Apple's libc++ (Xcode 26 / Apple Clang 21) does not, so this header
 *  has a minimal fallback for the plain "R(Args...)" signature. Delete the
 *  fallback once every supported compiler defines
 *  __cpp_lib_move_only_function.
 *
 */

#ifndef PokemonAutomation_MoveOnlyFunction_H
#define PokemonAutomation_MoveOnlyFunction_H

#include <version>
#include <functional>

#ifndef __cpp_lib_move_only_function
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#endif

namespace PokemonAutomation{



#ifdef __cpp_lib_move_only_function

template <typename Signature>
using MoveOnlyFunction = std::move_only_function<Signature>;

#else

template <typename Signature>
class MoveOnlyFunction;

template <typename R, typename... Args>
class MoveOnlyFunction<R(Args...)>{
public:
    MoveOnlyFunction() noexcept = default;
    MoveOnlyFunction(std::nullptr_t) noexcept {}

    template <typename Function>
        requires (
            !std::is_same_v<std::remove_cvref_t<Function>, MoveOnlyFunction> &&
            std::is_invocable_r_v<R, std::decay_t<Function>&, Args...>
        )
    MoveOnlyFunction(Function&& function){
        using Stored = std::decay_t<Function>;
        //  Like std::move_only_function, a null function pointer makes an
        //  empty wrapper.
        if constexpr (std::is_pointer_v<Stored> || std::is_member_pointer_v<Stored>){
            if (function == nullptr){
                return;
            }
        }
        m_callable = std::make_unique<Holder<Stored>>(std::forward<Function>(function));
    }

    MoveOnlyFunction(MoveOnlyFunction&&) noexcept = default;
    MoveOnlyFunction& operator=(MoveOnlyFunction&&) noexcept = default;
    MoveOnlyFunction(const MoveOnlyFunction&) = delete;
    MoveOnlyFunction& operator=(const MoveOnlyFunction&) = delete;

    explicit operator bool() const noexcept{
        return m_callable != nullptr;
    }
    friend bool operator==(const MoveOnlyFunction& function, std::nullptr_t) noexcept{
        return !function;
    }

    //  Calling an empty wrapper is undefined, as for std::move_only_function.
    R operator()(Args... args){
        return m_callable->call(std::forward<Args>(args)...);
    }


private:
    class Callable{
    public:
        virtual ~Callable() = default;
        virtual R call(Args&&... args) = 0;
    };

    template <typename Stored>
    class Holder final : public Callable{
    public:
        template <typename Source>
        explicit Holder(Source&& source)
            : m_stored(std::forward<Source>(source))
        {}
        virtual R call(Args&&... args) override{
            return std::invoke_r<R>(m_stored, std::forward<Args>(args)...);
        }

    private:
        Stored m_stored;
    };

    std::unique_ptr<Callable> m_callable;
};

#endif



}
#endif
