/*  Aligned Malloc
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Copy-edited from:
 *      https://github.com/Mysticial/y-cruncher/blob/master/trunk/Source/PublicLibs/BasicLibs/Memory/AlignedMalloc.cpp
 *
 */

#include <string.h>
#include <stdlib.h>
#include <new>
#include "Common/Cpp/Exceptions.h"
#include "AlignedMalloc.h"

#define PA_ENABLE_MALLOC_CHECKING
//#define PA_ZERO_INITIALIZE

const size_t BUFFER_CHECK_BOT = (size_t)0xea097be64bd0187d;
const size_t BUFFER_CHECK_TOP = (size_t)0xc72bf73f0559cfe4;

namespace PokemonAutomation{


void* aligned_malloc(size_t bytes, size_t alignment){
    if (alignment < sizeof(size_t)){
        alignment = sizeof(size_t);
    }
#ifdef PA_ENABLE_MALLOC_CHECKING
    if ((alignment & (alignment - 1)) != 0){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Alignment must be a power of two.");
    }
#endif

    size_t actual_bytes = bytes + alignment + sizeof(size_t)*4;
    void* free_ptr = malloc(actual_bytes);
    if (free_ptr == nullptr){
        throw std::bad_alloc();
//        return nullptr;
    }
#ifdef PA_ZERO_INITIALIZE
    memset(free_ptr, 0, actual_bytes);
#endif
    size_t free_address = (size_t)free_ptr;
    size_t min_ret_address = free_address + sizeof(size_t)*3;

    //  Align
    size_t ret_address = min_ret_address;
    ret_address &= ~(size_t)(alignment - 1);
    ret_address += alignment;

    size_t* ret = (size_t*)ret_address;
    ret[-3] = free_address;

#ifdef PA_ENABLE_MALLOC_CHECKING
    ret[-2] = bytes;
    ret[-1] = BUFFER_CHECK_BOT;
    memcpy((char*)ret + bytes, &BUFFER_CHECK_TOP, sizeof(size_t));
#endif

    return ret;
}
void aligned_free(void* ptr){
    if (ptr == nullptr){
        return;
    }

    check_aligned_ptr(ptr);

    size_t* ret = (size_t*)ptr;
    size_t free_int = ret[-3];

    ptr = (void*)free_int;
    free(ptr);
}
void check_aligned_ptr(const void* ptr){
#ifdef PA_ENABLE_MALLOC_CHECKING
    if (ptr == nullptr){
        return;
    }

    const size_t* ret = (const size_t*)ptr;
//    size_t free_int = ret[-3];

    size_t bytes = ret[-2];
    size_t check = ret[-1];
    if (check != BUFFER_CHECK_BOT){
//        std::terminate();
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Memory buffer has been underrun.");
    }
    memcpy(&check, (const char*)ptr + bytes, sizeof(size_t));
    if (check != BUFFER_CHECK_TOP){
//        std::terminate();
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Memory buffer has been overrun.");
    }
#endif
}


}
