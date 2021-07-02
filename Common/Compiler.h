/*  Compiler Specifics
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Compiler_H
#define PokemonAutomation_Compiler_H

#include <stddef.h>

namespace PokemonAutomation{



#if 0
#elif _MSC_VER

using ssize_t = ptrdiff_t;

#pragma warning(disable:4100)   //  Unreferenced Formal Parameter
#pragma warning(disable:4127)   //  Conditional expresstion is constant
#pragma warning(disable:4996)   //  Unsafe function


#define __PRETTY_FUNCTION__ __FUNCSIG__


#elif __GNUC__

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"


#else
#error "Unsupported Compiler."
#endif




}
#endif
