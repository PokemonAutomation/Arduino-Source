/*  Crash Dump
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <time.h>
#include <fstream>
#include "Common/Cpp/Unicode.h"
#include "Common/Cpp/PrettyPrint.h"
//#include "ClientSource/Libraries/Logging.h"
#include "CrashDump.h"

#include <iostream>
using std::cout;
using std::endl;



#if _WIN32 && _MSC_VER
#pragma comment (lib, "Dbghelp.lib")
#include <Windows.h>
#include <winioctl.h>
#include <Dbghelp.h>
namespace PokemonAutomation{


long WINAPI crash_handler(EXCEPTION_POINTERS* e){
    static bool handled = false;
    if (handled){
        return EXCEPTION_CONTINUE_SEARCH;
    }
    handled = true;

//    cout << "Oops... Program has crashed." << endl;
//    cout << "Creating mini-dump file..." << endl;

    _wmkdir(L"ErrorDumps");

    std::string filename = "ErrorDumps/SerialPrograms-";
    filename += now_to_filestring();

    std::ofstream log;
    log.open(filename + ".log");

    log << "Oops... Program has crashed." << endl;
    log << "Creating mini-dump file..." << endl;

    HANDLE handle = CreateFileW(
        utf8_to_wstr(filename + ".dmp").c_str(),
        FILE_WRITE_ACCESS,
        FILE_SHARE_READ,
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        0
    );
    if (handle == INVALID_HANDLE_VALUE){
        DWORD error = GetLastError();
//        cout << "Unable to create dump file: " << error << endl;
        log << "Unable to create dump file: " << error << endl;
        return EXCEPTION_EXECUTE_HANDLER;
    }

    MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
    exceptionInfo.ThreadId = GetCurrentThreadId();
    exceptionInfo.ExceptionPointers = e;
    exceptionInfo.ClientPointers = FALSE;

    int ret = MiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        handle,
        MiniDumpNormal,
        e != nullptr ? &exceptionInfo : nullptr,
        nullptr,
        nullptr
    );
    CloseHandle(handle);

    if (!ret){
        DWORD error = GetLastError();
//        cout << "Unable to create minidump: " << error << endl;
        log << "Unable to create minidump: " << error << endl;
    }else{
//        cout << "Minidump created!" << endl;
        log << "Minidump created!" << endl;
    }

    return EXCEPTION_CONTINUE_SEARCH;
}



void setup_crash_handler(){
    SetUnhandledExceptionFilter(crash_handler);
}


}
#else
namespace PokemonAutomation{

void setup_crash_handler(){
    //  Not supported
}

}
#endif
