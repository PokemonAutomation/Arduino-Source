/*  Guard Page Buffer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <new>
#include "Common/Cpp/Exceptions.h"
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
    release();
}
GuardPageMemory::GuardPageMemory(GuardPageMemory&& x) noexcept
    : m_base(x.m_base)
    , m_mapped_bytes(x.m_mapped_bytes)
    , m_data(x.m_data)
{
    x.m_base = nullptr;
    x.m_mapped_bytes = 0;
    x.m_data = nullptr;
}
void GuardPageMemory::operator=(GuardPageMemory&& x) noexcept{
    if (this == &x){
        return;
    }
    release();
    m_base = x.m_base;
    m_mapped_bytes = x.m_mapped_bytes;
    m_data = x.m_data;
    x.m_base = nullptr;
    x.m_mapped_bytes = 0;
    x.m_data = nullptr;
}
void GuardPageMemory::release() noexcept{
    if (m_base == nullptr){
        return;
    }
#ifdef _WIN32
    VirtualFree(m_base, 0, MEM_RELEASE);
#else
    munmap(m_base, m_mapped_bytes);
#endif
    m_base = nullptr;
}

GuardPageMemory::GuardPageMemory(size_t bytes, size_t alignment){
    if (alignment == 0 || (alignment & (alignment - 1)) != 0){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Alignment must be a power of two.");
    }

    //  Room for the data plus the worst-case rounding of its start.
    size_t page = page_size();
    size_t data_pages = (bytes + alignment - 1 + page - 1) / page;
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

    m_data = (void*)((size_t)(guard - bytes) & ~(alignment - 1));
}



}
