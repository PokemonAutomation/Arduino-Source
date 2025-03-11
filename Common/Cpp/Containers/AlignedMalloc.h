/*  Aligned Malloc
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AlignedMalloc_H
#define PokemonAutomation_AlignedMalloc_H

#include <stddef.h>

namespace PokemonAutomation{


void* aligned_malloc(size_t bytes, size_t alignment);
void aligned_free(void* ptr);
void check_aligned_ptr(const void *ptr);


}
#endif
