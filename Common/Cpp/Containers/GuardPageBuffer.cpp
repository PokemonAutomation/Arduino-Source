/*  Guard Page Buffer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <new>
#include "GuardPageBuffer.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

namespace PokemonAutomation{



static size_t page_size(){
#ifdef _WIN32
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwPageSize;
#else
    return (size_t)sysconf(_SC_PAGESIZE);
#endif
}



GuardPageMemory::~GuardPageMemory(){
#ifdef _WIN32
    VirtualFree(m_base, 0, MEM_RELEASE);
#else
    munmap(m_base, m_mapped_bytes);
#endif
}
GuardPageMemory::GuardPageMemory(size_t bytes){
    size_t page = page_size();
    size_t data_pages = (bytes + page - 1) / page;
    m_mapped_bytes = (data_pages + 1) * page;

#ifdef _WIN32
    m_base = VirtualAlloc(nullptr, m_mapped_bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (m_base == nullptr){
        throw std::bad_alloc();
    }
    char* guard = (char*)m_base + data_pages * page;
    DWORD old_protect;
    if (!VirtualProtect(guard, page, PAGE_NOACCESS, &old_protect)){
        VirtualFree(m_base, 0, MEM_RELEASE);
        throw std::bad_alloc();
    }
#else
    m_base = mmap(nullptr, m_mapped_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (m_base == MAP_FAILED){
        throw std::bad_alloc();
    }
    char* guard = (char*)m_base + data_pages * page;
    if (mprotect(guard, page, PROT_NONE) != 0){
        munmap(m_base, m_mapped_bytes);
        throw std::bad_alloc();
    }
#endif

    m_data = guard - bytes;
}



}
