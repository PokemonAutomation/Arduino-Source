/*  Compiler Specifics
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Compiler_H
#define PokemonAutomation_Compiler_H

//#include <stddef.h>
namespace PokemonAutomation{


#define PA_ALIGNMENT    64


#if 0
#elif _MSC_VER

#define PA_NO_INLINE        __declspec(noinline)
#define PA_FORCE_INLINE     inline __forceinline
template <typename type> using r_ptr  = type *__restrict;
template <typename type> using c_ptr  = type const*__restrict;
template <typename type> using r_ref  = type &__restrict;
template <typename type> using c_ref  = type const&__restrict;
template <typename type> using r_rref = type &&__restrict;

//using ssize_t = ptrdiff_t;

#pragma warning(disable:4100)   //  Unreferenced Formal Parameter
#pragma warning(disable:4127)   //  Conditional expresstion is constant
#pragma warning(disable:4324)   //  structure was padded due to alignment specifier
#pragma warning(disable:4458)   //  Hiding of class members
#pragma warning(disable:4996)   //  Unsafe function

#define PA_CURRENT_FUNCTION __FUNCSIG__


#elif __GNUC__

#define PA_NO_INLINE        __attribute__ ((noinline))
#define PA_FORCE_INLINE     inline __attribute__ ((always_inline))
template <typename type> using r_ptr  = type *__restrict__;
template <typename type> using c_ptr  = type const*__restrict__;
template <typename type> using r_ref  = type &__restrict__;
template <typename type> using c_ref  = type const&__restrict__;
template <typename type> using r_rref = type &&__restrict__;

// Align a struct on x-byte boundary.
// It's the minimum alignment for a struct or a struct member.
#define PA_ALIGN_STRUCT(x) __attribute__((aligned(x)))

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

#define PA_CURRENT_FUNCTION __PRETTY_FUNCTION__


#else
#error "Unsupported Compiler."
#endif




}
#endif
