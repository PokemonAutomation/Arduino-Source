/*  Compiler Specifics
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Compiler_H
#define PokemonAutomation_Compiler_H

//#include <stddef.h>
#ifdef __cplusplus
namespace PokemonAutomation{
#endif


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

#define PA_CURRENT_FUNCTION __FUNCSIG__


#elif __GNUC__

#define PA_NO_INLINE        __attribute__ ((noinline))
#define PA_FORCE_INLINE     inline __attribute__ ((always_inline))
template <typename type> using r_ptr  = type *__restrict__;
template <typename type> using c_ptr  = type const*__restrict__;
template <typename type> using r_ref  = type &__restrict__;
template <typename type> using c_ref  = type const&__restrict__;
template <typename type> using r_rref = type &&__restrict__;

#define PA_CURRENT_FUNCTION __PRETTY_FUNCTION__


#else
#error "Unsupported Compiler."
#endif




#ifdef __cplusplus
}
#endif
#endif
