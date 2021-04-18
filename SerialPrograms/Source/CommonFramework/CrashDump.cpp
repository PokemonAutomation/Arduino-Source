/*  Crash Dump
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <time.h>
#include "Common/Clientside/Unicode.h"
//#include "ClientSource/Libraries/Logging.h"
#include "CrashDump.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

std::string now_to_filestring(){
#if _WIN32 && _MSC_VER
#pragma warning(disable:4996)
#endif

    time_t t = time(0);
    struct tm* now = localtime(&t);

    std::string str;
    str += std::to_string(now->tm_year + 1900);
    str += std::string(now->tm_mon + 1 < 10 ? "0" : "") + std::to_string(now->tm_mon + 1);
    str += std::string(now->tm_mday    < 10 ? "0" : "") + std::to_string(now->tm_mday);
    str += "-";
    str += std::string(now->tm_hour    < 10 ? "0" : "") + std::to_string(now->tm_hour);
    str += std::string(now->tm_min     < 10 ? "0" : "") + std::to_string(now->tm_min);
    str += std::string(now->tm_sec     < 10 ? "0" : "") + std::to_string(now->tm_sec);
    return str;
}

}


#if _WIN32
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

    cout << "Oops... Program has crashed." << endl;
    cout << "Creating mini-dump file..." << endl;

    std::string filename = "SerialPrograms-";
    filename += now_to_filestring();
    filename += ".dmp";

    HANDLE handle = CreateFileW(
        utf8_to_wstr(filename).c_str(),
        FILE_WRITE_ACCESS,
        FILE_SHARE_READ,
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        0
    );
    if (handle == INVALID_HANDLE_VALUE){
        cout << "Unable to create dump file: " << GetLastError() << endl;
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
        cout << "Unable to create minidump: " << GetLastError() << endl;
    }else{
        cout << "Minidump created!" << endl;
    }

    return EXCEPTION_CONTINUE_SEARCH;
}



void setup_crash_handler(){
    SetUnhandledExceptionFilter(crash_handler);
}


}
#else
void setup_crash_handler(){
    //  Not supported
}
#endif
