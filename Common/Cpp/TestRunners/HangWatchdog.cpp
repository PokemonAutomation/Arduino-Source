/*  Hang Watchdog
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <sstream>
#include <thread>
#include <vector>

#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/Thread.h"
#include "HangWatchdog.h"


#if defined(_WIN32)

#include <windows.h>
#include <psapi.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

#else

#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <execinfo.h>
#include <dlfcn.h>
#include <cxxabi.h>

#if defined(__linux__)
#include <dirent.h>
#include <sys/syscall.h>
#endif

#endif



namespace PokemonAutomation{
namespace HangWatchdog{



// ---------------------------------------------------------------------------
//  Raw stderr output.
//
//  The dump goes to the stderr file descriptor/handle directly, not stdio:
//  when output is piped (as in CI) the stdio buffers are lost when the
//  process is terminated, and the app may have redirected std::cerr into its
//  logger.
// ---------------------------------------------------------------------------

static void raw_stderr_write(const char* data, size_t n){
    if (n == 0){
        return;
    }
    #if defined(_WIN32)
        HANDLE handle = GetStdHandle(STD_ERROR_HANDLE);
        if (handle != nullptr && handle != INVALID_HANDLE_VALUE){
            DWORD written = 0;
            WriteFile(handle, data, (DWORD)n, &written, nullptr);
        }
    #else
        if (::write(2, data, n) < 0){
            //  Nothing more we can do.
        }
    #endif
}

static void raw_stderr_line(const std::string& line){
    raw_stderr_write(line.data(), line.size());
}

//  The id of the calling thread, for labels and for skipping ourselves.
//  On Linux use the kernel tid directly: some glibc builds use an opaque
//  (pointer) pthread_t, and pthread_self() is only the tid on standard
//  builds.
#if !defined(_WIN32)
static uintptr_t my_thread_id(){
    #if defined(__linux__)
        return (uintptr_t)syscall(SYS_gettid);
    #else
        return (uintptr_t)pthread_self();
    #endif
}
#endif



// ---------------------------------------------------------------------------
//  Thread registry - declared here, defined in the stack-dump section
//  below. register_thread() calls the platform one.
// ---------------------------------------------------------------------------

#if defined(_WIN32)
void register_thread_windows();
#else
void register_thread_posix();
#endif


// ---------------------------------------------------------------------------
//  Shared state
// ---------------------------------------------------------------------------

namespace{


struct State{
    Mutex lock;
    bool running = false;
    WallDuration timeout = WallDuration::zero();
    WallClock start_time{};
    WallClock last_progress{};
    std::atomic<uint64_t> completed_count{0};
    std::function<std::string()> state_provider;
    Thread thread;
};

State& state(){
    static State s;
    return s;
}

//  PA_HANG_WATCHDOG: integer seconds; 0 (or negative) disables the watchdog.
WallDuration configured_timeout(WallDuration default_timeout){
    const char* env = ::getenv("PA_HANG_WATCHDOG");
    if (env != nullptr){
        char* end = nullptr;
        long seconds = std::strtol(env, &end, 10);
        if (end != env){
            if (seconds <= 0){
                return WallDuration::zero();
            }
            return std::chrono::seconds(seconds);
        }
    }
    return default_timeout;
}



}  // namespace


void register_thread(){
    #if defined(_WIN32)
        register_thread_windows();
    #else
        register_thread_posix();
    #endif
}



// ---------------------------------------------------------------------------
//  All-thread stack dump: Windows
//
//  Suspend every registered thread, walk its stack with dbghelp's
//  StackWalk64, symbolize with SymFromAddr, resume it, move on. Frames come
//  out as "symbol + module_offset (absolute_address)"; without PDBs the
//  offset is still enough to locate the function in the binary.
//
//  Threads are enumerated from the program's own registry
//  (register_thread()) rather than with the toolhelp snapshot API: the
//  THREADLIST32-based enumeration is not available in every SDK/MSVC
//  configuration. Library-created threads (Qt, ONNX...) that the program
//  never registered are not captured - noted at the end, as on macOS.
// ---------------------------------------------------------------------------

#if defined(_WIN32)

//  Registry of thread ids created by the program (see register_thread()).
//  On Windows this is the source of the dump's thread list.
static Mutex g_registry_lock_windows;
static std::vector<DWORD> g_registry_windows;

namespace{

constexpr int kMaxFrames = 48;

DWORD native_machine_type(){
    #if defined(_M_X64)
        return IMAGE_FILE_MACHINE_AMD64;
    #elif defined(_M_ARM64)
        return IMAGE_FILE_MACHINE_ARM64;
    #else
        return IMAGE_FILE_MACHINE_I386;
    #endif
}

void dump_all_thread_stacks_windows(){
    HANDLE h_process = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        FALSE,
        GetCurrentProcessId()
    );
    if (h_process == nullptr){
        raw_stderr_line("Stack dump: OpenProcess failed (error " + std::to_string((int)::GetLastError()) + ")\n");
        return;
    }
    SymSetOptions(SymGetOptions() | SYMOPT_UNDNAME);
    SymInitialize(h_process, nullptr, TRUE);

    //  Loaded module ranges: used by the stack-peek fallback below
    //  to name the module of candidate return addresses when
    //  StackWalk64 cannot produce frames (no PDB in CI).
    struct ModuleRange{
        uintptr_t base;
        size_t size;
        char name[64];
    };
    std::vector<ModuleRange> modules;
    {
        HMODULE handles[1024];
        DWORD needed = 0;
        if (EnumProcessModules(h_process, handles, sizeof(handles), &needed)){
            for (DWORD i = 0; i * sizeof(HMODULE) < needed; i++){
                ModuleRange r;
                r.base = (uintptr_t)handles[i];
                r.size = 0;
                r.name[0] = '\0';
                GetModuleBaseNameA(h_process, handles[i], r.name, sizeof(r.name));
                IMAGEHLP_MODULE64 mod;
                std::memset(&mod, 0, sizeof(mod));
                mod.SizeOfStruct = sizeof(mod);
                if (SymGetModuleInfo64(h_process, (DWORD64)r.base, &mod)){
                    r.size = mod.SizeOfImage;
                }
                modules.push_back(r);
            }
        }
    }

    std::vector<DWORD> tids;
    {
        std::lock_guard<Mutex> lg(g_registry_lock_windows);
        tids = g_registry_windows;
    }

    DWORD self_tid = GetCurrentThreadId();
    DWORD machine = native_machine_type();
    int captured = 0;

    for (DWORD tid : tids){
        if (tid == self_tid){
            continue;  //  Don't suspend the thread that is doing the dump.
        }

        HANDLE h_thread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT, FALSE, tid);
        if (h_thread == nullptr){
            continue;
        }

        DWORD suspend_count = SuspendThread(h_thread);
        if (suspend_count == 0xFFFFFFFF){
            CloseHandle(h_thread);
            continue;
        }

        CONTEXT ctx;
        std::memset(&ctx, 0, sizeof(ctx));
        ctx.ContextFlags = CONTEXT_FULL;
        if (GetThreadContext(h_thread, &ctx)){
            STACKFRAME64 sf;
            std::memset(&sf, 0, sizeof(sf));
            #if defined(_M_ARM64) || defined(_M_ARM)
                //  The "Machine" field only exists in the ARM STACKFRAME64.
                sf.Machine = machine;
            #endif
            #if defined(_M_X64) || defined(_M_ARM64)
                sf.AddrPC.Offset = ctx.Rip;
                sf.AddrFrame.Offset = ctx.Rbp;
                sf.AddrStack.Offset = ctx.Rsp;
            #else
                sf.AddrPC.Offset = ctx.Eip;
                sf.AddrFrame.Offset = ctx.Ebx;
                sf.AddrStack.Offset = ctx.Esp;
            #endif

            char header[80];
            int len = std::snprintf(
                header,
                sizeof(header),
                "\n===== Thread 0x%lx =====\n",
                (unsigned long)tid
            );
            if (len > 0){
                raw_stderr_write(header, (size_t)len);
            }

            int depth = 0;
            while (depth < kMaxFrames &&
                StackWalk64(
                    machine,
                    h_process,
                    h_thread,
                    &sf,
                    &ctx,
                    nullptr,              //  ReadProcessMemory
                    nullptr,              //  GetFunctionTable
                    SymGetModuleBase64,   //  GetModuleBase
                    nullptr                 //  TranslateAddress
                )
            ){
                if (sf.AddrPC.Offset == 0){
                    break;
                }

                SYMBOL_INFO sym;
                std::memset(&sym, 0, sizeof(sym));
                sym.SizeOfStruct = sizeof(sym);
                sym.MaxNameLen = sizeof(sym.Name) - 1;
                DWORD64 displacement = 0;
                const char* name = "??";
                if (SymFromAddr(h_process, sf.AddrPC.Offset, &displacement, &sym)){
                    name = sym.Name;
                }

                DWORD64 module_base = SymGetModuleBase64(h_process, sf.AddrPC.Offset);
                char line[600];
                int line_len = std::snprintf(
                    line,
                    sizeof(line),
                    "    #%02d  %s + 0x%016llX  (0x%016llX)\n",
                    depth,
                    name,
                    (unsigned long long)(sf.AddrPC.Offset - module_base),
                    (unsigned long long)sf.AddrPC.Offset
                );
                if (line_len > 0){
                    raw_stderr_write(line, (size_t)line_len);
                }
                depth++;
            }
            if (depth == 0){
                //  Diagnostics: report why the walk produced nothing.
                DWORD last_error = ::GetLastError();
                char line[256];
                int line_len = std::snprintf(
                    line,
                    sizeof(line),
                    "    (no frames captured; StackWalk64 error %lu, Rip 0x%016llX)\n",
                    (unsigned long)last_error,
                    (unsigned long long)ctx.Rip
                );
                if (line_len > 0){
                    raw_stderr_write(line, (size_t)line_len);
                }

                //  Fallback when the walk produced nothing (in CI the
                //  build has no PDB, so StackWalk64 often cannot even
                //  step out of the first kernel wait): scan the top of
                //  the suspended thread's stack and report, for each
                //  candidate value that points into executable memory,
                //  which loaded module it is in. No function names are
                //  possible without symbols, but the module pattern
                //  (e.g. SerialPrograms.exe / Qt6Network.dll / ntdll.dll)
                //  already identifies where the thread is parked.
                uintptr_t sp = (uintptr_t)ctx.Rsp;
                int shown = 0;
                for (int i = 0; i < 24 && shown < 10; i++){
                    UINT64 value;
                    SIZE_T got = 0;
                    if (!::ReadProcessMemory(h_process, (LPCVOID)sp, &value, sizeof(value), &got)
                        || got != sizeof(value)){
                        break;
                    }
                    sp += sizeof(value);
                    if (value == 0){
                        continue;
                    }
                    MEMORY_BASIC_INFORMATION mbi;
                    if (::VirtualQueryEx(h_process, (LPCVOID)value, &mbi, sizeof(mbi)) != sizeof(mbi)){
                        continue;
                    }
                    if (mbi.State != MEM_COMMIT){
                        continue;
                    }
                    const DWORD protect = mbi.Protect & 0xFF;
                    if ((protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)) == 0){
                        continue;
                    }
                    for (const ModuleRange& m : modules){
                        if (value >= m.base && value < m.base + (uintptr_t)m.size){
                            char peek[320];
                            int peek_len = std::snprintf(
                                peek,
                                sizeof(peek),
                                "    stack[%02d]  %s + 0x%016llx  (0x%016llx)\n",
                                shown,
                                m.name,
                                (unsigned long long)(value - m.base),
                                (unsigned long long)value
                            );
                            if (peek_len > 0){
                                raw_stderr_write(peek, (size_t)peek_len);
                            }
                            shown++;
                            break;
                        }
                    }
                }
            }
            captured++;
        }

        ResumeThread(h_thread);
        CloseHandle(h_thread);
    }

    if (captured == 0){
        raw_stderr_line("Stack dump: no threads captured\n");
    }else{
        char note[160];
        int len = std::snprintf(
            note,
            sizeof(note),
            "(note: %d of %d registered threads captured; library-created threads are not registered)\n",
            captured,
            (int)tids.size()
        );
        if (len > 0){
            raw_stderr_write(note, (size_t)len);
        }
    }

    SymCleanup(h_process);
    CloseHandle(h_process);
}

}  // namespace

void register_thread_windows(){
    std::lock_guard<Mutex> lg(g_registry_lock_windows);
    DWORD self = GetCurrentThreadId();
    for (DWORD tid : g_registry_windows){
        if (tid == self){
            return;
        }
    }
    g_registry_windows.push_back(self);
}

#else  // !defined(_WIN32)


// ---------------------------------------------------------------------------
//  All-thread stack dump: Linux/macOS
//
//  Install a handler for SIGUSR2 that captures the *receiving* thread's
//  backtrace() into a fixed slot, then send SIGUSR2 to every other thread
//  in the process. Threads blocked in a syscall (futex wait, mutex lock,
//  file I/O...) are interrupted and run the handler, so this captures
//  exactly where each thread is parked. The frames are symbolized later,
//  on the watchdog thread, with backtrace_symbols_fd().
// ---------------------------------------------------------------------------

namespace{

constexpr int kSignal = SIGUSR2;
constexpr int kMaxThreads = 512;
constexpr int kMaxFrames = 48;
constexpr int kWaitIterations = 200;  //  200 x 10ms = up to 2s

struct Slot{
    std::atomic<bool> done{false};
    uintptr_t tid = 0;
    int n = 0;
    void* frames[kMaxFrames];
};

//  Registry of threads created by the program: the real pthread handle plus
//  the label id recorded at creation. On macOS (and on Linux builds where
//  pthread_t is an opaque value, not the kernel tid) the dump signals these
//  handles, because a /proc tid cannot be cast to pthread_t there.
struct RegistryEntry{
    pthread_t handle;
    uintptr_t label;
};
static Mutex g_registry_lock;
static std::vector<RegistryEntry> g_registry;

void registry_push(){
    std::lock_guard<Mutex> lg(g_registry_lock);
    pthread_t self = pthread_self();
    uintptr_t label = my_thread_id();
    for (const auto& entry : g_registry){
        if (entry.label == label){
            return;
        }
    }
    g_registry.emplace_back(self, label);
}

static Slot g_slots[kMaxThreads];
static std::atomic<int> g_slot_count{0};
//  Per-thread re-entrancy guard. While a thread is inside the handler,
//  SIGUSR2 is masked for that thread, so a second delivery to it cannot
//  normally happen; this guards the pathological case. A process-wide
//  guard would be wrong: it would drop captures from other threads that
//  happen to run their handler at the same time.
static thread_local bool g_in_handler = false;

static void hang_signal_handler(int){
    if (g_in_handler){
        return;
    }
    g_in_handler = true;
    int slot = g_slot_count.fetch_add(1, std::memory_order_acq_rel);
    if (slot < kMaxThreads){
        Slot& s = g_slots[slot];
        s.tid = my_thread_id();
        s.n = backtrace(s.frames, kMaxFrames);
        s.done.store(true, std::memory_order_release);
    }
    g_in_handler = false;
}

//  Scheduling policy/priority of a thread in the process, via
//  pidfd_open() + sched_getattr(). Declared by hand (the sandbox
//  toolchain ships a reduced glibc header set without <sys/sched.h>;
//  the symbols are in glibc >= 2.30/2.34, which every CI image has).
#if defined(__linux__)
struct sched_attr_min{
    uint32_t size;
    uint32_t sched_policy;
    uint64_t sched_flags;
    int32_t sched_nice;
    int32_t sched_priority;
    uint64_t sched_runtime;
    uint64_t sched_deadline;
    uint64_t sched_period;
};
//  Syscall numbers (stable across kernel and glibc versions; the sandbox
//  ships a reduced glibc without sched_getattr, so call the kernel
//  directly).
#if defined(__aarch64__)
    static const long sys_pidfd_open_nr = 434;
    static const long sys_sched_getattr_nr = 416;
#elif defined(__x86_64__)
    static const long sys_pidfd_open_nr = 434;
    static const long sys_sched_getattr_nr = 310;
#else
    static const long sys_pidfd_open_nr = -1;
    static const long sys_sched_getattr_nr = -1;
#endif
void thread_sched_info(uintptr_t tid, char* buf, size_t n){
    if (sys_sched_getattr_nr < 0){
        std::snprintf(buf, n, "sched=?");
        return;
    }
    int pidfd = (int)syscall(sys_pidfd_open_nr, (long)tid, 0L);
    if (pidfd < 0){
        std::snprintf(buf, n, "sched=?");
        return;
    }
    struct sched_attr_min attr;
    std::memset(&attr, 0, sizeof(attr));
    attr.size = sizeof(attr);
    int ret = (int)syscall(sys_sched_getattr_nr, (long)pidfd, (long)&attr, (long)sizeof(attr), 0L);
    ::close(pidfd);
    if (ret != 0){
        std::snprintf(buf, n, "sched=?");
        return;
    }
    const char* policy = "OTHER";
    if (attr.sched_policy == 2){
        policy = "RR";
    }else if (attr.sched_policy == 1){
        policy = "FIFO";
    }
    std::snprintf(buf, n, "sched=%s prio=%d nice=%d", policy, (int)attr.sched_priority, (int)attr.sched_nice);
}
#else
void thread_sched_info(uintptr_t, char* buf, size_t n){
    std::snprintf(buf, n, "");
}
#endif

std::vector<uintptr_t> enumerate_thread_ids(){
    std::vector<uintptr_t> ret;
    #if defined(__linux__)
        //  /proc/self/task lists every thread id in the process.
        DIR* dir = ::opendir("/proc/self/task");
        if (dir != nullptr){
            dirent* ent = nullptr;
            while ((ent = ::readdir(dir)) != nullptr){
                if (ent->d_name[0] >= '1' && ent->d_name[0] <= '9'){
                    ret.push_back((uintptr_t)std::strtoul(ent->d_name, nullptr, 10));
                }
            }
            ::closedir(dir);
        }
    #else
        //  macOS: use the registry filled by register_thread().
        static Mutex lock;
        static std::vector<pthread_t> threads;
        std::lock_guard<Mutex> lg(lock);
        for (pthread_t t : threads){
            ret.push_back((uintptr_t)t);
        }
    #endif
    return ret;
}

void dump_all_thread_stacks_posix(){
    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler = hang_signal_handler;
    sigemptyset(&sa.sa_mask);
    //  No SA_NODEFER: while a thread is in the handler the signal is masked
    //  for it, so a process-directed signal (the fallback below) is delivered
    //  to a *different* thread each time - which is what makes the fallback
    //  reach every thread in turn.
    sa.sa_flags = SA_RESTART;
    ::sigaction(kSignal, &sa, nullptr);

    g_slot_count.store(0, std::memory_order_relaxed);
    for (auto& slot : g_slots){
        slot.done.store(false, std::memory_order_relaxed);
    }

    uintptr_t self = my_thread_id();

    //  How to signal the other threads:
    //
    //  - Standard Linux: pthread_t IS the kernel tid, so signal every thread
    //    listed in /proc/self/task directly - including threads created by
    //    libraries (Qt) that the program never registered.
    //  - macOS, and non-standard Linux builds where pthread_t is an opaque
    //    value: a /proc tid cannot be cast to pthread_t, so signal the
    //    threads registered via register_thread() instead. Library threads
    //    that did not register will not be captured (noted at the end).
    bool tid_is_pthread_t = false;
    #if defined(__linux__)
        tid_is_pthread_t = (my_thread_id() == (uintptr_t)pthread_self());
    #endif

    int signaled = 0;
    if (tid_is_pthread_t){
        for (uintptr_t tid : enumerate_thread_ids()){
            if (tid == self){
                continue;
            }
            if (::pthread_kill((pthread_t)tid, kSignal) == 0){
                signaled++;
            }
        }
    }else{
        std::lock_guard<Mutex> lg(g_registry_lock);
        for (const auto& entry : g_registry){
            if (entry.label == self){
                continue;
            }
            if (::pthread_kill(entry.handle, kSignal) == 0){
                signaled++;
            }
        }
    }
    int expected = signaled;

    //  Wait for the handlers to run. A few threads may have exited between
    //  enumeration and delivery.
    for (int i = 0; i < kWaitIterations && g_slot_count.load(std::memory_order_acquire) < expected; i++){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    int n = g_slot_count.load(std::memory_order_acquire);
    for (int c = 0; c < n; c++){
        Slot& s = g_slots[c];
        if (!s.done.load(std::memory_order_acquire)){
            continue;
        }
        #if defined(__linux__)
            char sched[160] = "";
            thread_sched_info(s.tid, sched, sizeof(sched));
        #else
            const char* sched = "";
        #endif
        char header[320];
        int len = std::snprintf(
            header,
            sizeof(header),
            "\n===== Thread 0x%lx (%s) =====\n",
            (unsigned long)s.tid,
            sched
        );
        if (len > 0){
            raw_stderr_write(header, (size_t)len);
        }
        //  Resolve each frame with dladdr(): the CI binaries keep
        //  their symbol tables (the build only drops -g/PDBs), so
        //  this names the function even without debug info.
        for (int f = 0; f < s.n; f++){
            Dl_info info;
            const char* sym = "??";
            char* demangled = nullptr;
            const char* module = "??";
            uintptr_t offset = 0;
            if (::dladdr(s.frames[f], &info)){
                if (info.dli_sname != nullptr){
                    sym = info.dli_sname;
                    demangled = abi::__cxa_demangle(sym, nullptr, nullptr, nullptr);
                    if (demangled != nullptr){
                        sym = demangled;
                    }
                }
                if (info.dli_fname != nullptr){
                    module = info.dli_fname;
                }
                if (info.dli_saddr != nullptr){
                    offset = (uintptr_t)s.frames[f] - (uintptr_t)info.dli_saddr;
                }
            }
            char line[700];
            int line_len = std::snprintf(
                line,
                sizeof(line),
                "    #%02d  %s (+0x%lx)  [%s] @ 0x%lx\n",
                f,
                sym,
                (unsigned long)offset,
                module,
                (unsigned long)(uintptr_t)s.frames[f]
            );
            if (line_len > 0){
                raw_stderr_write(line, (size_t)line_len);
            }
            ::free(demangled);
        }
    }

    if (n < expected){
        char note[128];
        int len = std::snprintf(
            note,
            sizeof(note),
            "(note: captured %d of %d threads; the rest may have exited or not received the signal)\n",
            n,
            expected
        );
        if (len > 0){
            raw_stderr_write(note, (size_t)len);
        }
    }

    ::signal(kSignal, SIG_DFL);
}

}  // namespace

void register_thread_posix(){
    registry_push();
}

#endif  // !defined(_WIN32)



// ---------------------------------------------------------------------------
//  The dump + terminate
// ---------------------------------------------------------------------------

namespace{


void dump_hang(WallDuration elapsed_since_progress, WallDuration timeout){
    using std::chrono::duration;
    using std::chrono::duration_cast;

    State& s = state();
    uint64_t completed = s.completed_count.load(std::memory_order_acquire);
    WallClock now = current_time();
    WallClock start_time = WallClock{};
    {
        std::lock_guard<Mutex> lg(s.lock);
        start_time = s.start_time;
    }

    std::ostringstream os;
    os << "===================================================================\n";
    os << "PA-HANG-WATCHDOG: zero progress - no test completed for "
       << duration_cast<duration<double>>(elapsed_since_progress).count()
       << "s (timeout: "
       << duration_cast<duration<double>>(timeout).count()
       << "s)\n";
    os << "Watchdog started "
       << duration_cast<duration<double>>(now - start_time).count()
       << "s ago. Tests completed so far: " << completed << "\n";
    os << "===================================================================\n";
    raw_stderr_line(os.str());

    //  1.  Registered state: runner counts, pool states, logger queues.
    std::function<std::string()> provider;
    {
        std::lock_guard<Mutex> lg(s.lock);
        provider = s.state_provider;
    }
    if (provider){
        try{
            raw_stderr_line(provider());
        }catch (...){
            raw_stderr_line("(state provider threw)\n");
        }
    }

    //  2.  Where every thread in the process is parked.
    raw_stderr_line("\n---- All-thread stack traces ----\n");
    try{
        #if defined(_WIN32)
            dump_all_thread_stacks_windows();
        #else
            dump_all_thread_stacks_posix();
        #endif
    }catch (...){
        raw_stderr_line("(stack dump threw)\n");
    }

    raw_stderr_line(
        "PA-HANG-WATCHDOG: terminating process (exit code " + std::to_string(EXIT_CODE) + ")\n"
    );
    std::_Exit(EXIT_CODE);
}

void thread_body(){
    register_thread();  //  Include the watchdog thread itself in the registry.
    while (true){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        State& s = state();
        bool running = false;
        WallDuration elapsed_since_progress = WallDuration::zero();
        WallDuration timeout = WallDuration::zero();
        {
            std::lock_guard<Mutex> lg(s.lock);
            running = s.running;
            if (running){
                elapsed_since_progress = current_time() - s.last_progress;
                timeout = s.timeout;
            }
        }
        if (!running){
            return;
        }
        if (elapsed_since_progress >= timeout){
            dump_hang(elapsed_since_progress, timeout);
            //  Not reached: dump_hang() terminates the process.
        }
    }
}


}  // namespace



// ---------------------------------------------------------------------------
//  Public API
// ---------------------------------------------------------------------------


void start(WallDuration no_progress_timeout){
    State& s = state();
    WallDuration effective = configured_timeout(no_progress_timeout);
    {
        std::lock_guard<Mutex> lg(s.lock);
        if (s.running || effective <= WallDuration::zero()){
            return;
        }
        s.timeout = effective;
        s.start_time = current_time();
        s.last_progress = s.start_time;
        s.completed_count.store(0, std::memory_order_relaxed);
        s.running = true;
    }
    register_thread();  //  Include the calling (main) thread in the registry.
    s.thread = Thread([]{
        thread_body();
    });
}

void stop(){
    State& s = state();
    bool was_running = false;
    {
        std::lock_guard<Mutex> lg(s.lock);
        was_running = s.running;
        s.running = false;
        s.state_provider = nullptr;
    }
    if (was_running){
        //  The thread exits within one 500ms tick of s.running going false.
        s.thread.join();
    }
}

void note_test_completed(){
    State& s = state();
    s.completed_count.fetch_add(1, std::memory_order_acq_rel);
    {
        std::lock_guard<Mutex> lg(s.lock);
        s.last_progress = current_time();
    }
}

void set_state_provider(std::function<std::string()>&& provider){
    State& s = state();
    std::lock_guard<Mutex> lg(s.lock);
    s.state_provider = std::move(provider);
}



}
}
