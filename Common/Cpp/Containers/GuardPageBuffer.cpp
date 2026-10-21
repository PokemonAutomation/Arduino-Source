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



//  Return the OS page size in bytes: the unit in which the OS maps memory and
//  sets access rights. It is 4 KB on x86 Windows and Linux, and 16 KB on
//  Apple Silicon macOS.
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

//  Take over the pages of "x". Afterwards "x" owns no memory, so its
//  destructor does nothing.
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

//  Unmap the pages if this object still owns them. The destructor calls it,
//  and so does the move assignment before it takes the pages of another
//  object.
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

//  Map the data pages and the guard page, then place the data so that it ends
//  at the guard page:
//  1. Round the data size up to whole pages. Add alignment - 1 bytes first,
//     so that rounding the start down to "alignment" stays inside the mapping.
//  2. Map all the pages as read-write (mmap() or VirtualAlloc()). New pages
//     from the OS are all zero, so the buffer starts as zero bytes.
//  3. Make the last page no-access (mprotect() with PROT_NONE, or
//     VirtualProtect() with PAGE_NOACCESS). This is the guard page.
//  4. Put the start of the data at guard - bytes, rounded down to "alignment".
//  Example with 4096-byte pages, bytes = 72 and alignment = 4: it maps 1 data
//  page and 1 guard page, and the data starts at byte 4024 of the first page,
//  so the data ends at byte 4096, where the guard page starts.
//  If a step fails, it unmaps what it mapped and throws std::bad_alloc.
GuardPageMemory::GuardPageMemory(size_t bytes, size_t alignment){
    if (alignment == 0 || (alignment & (alignment - 1)) != 0){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Alignment must be a power of two.");
    }

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
