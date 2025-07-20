/*  Program Dumper (Windows)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#pragma comment (lib, "Dbghelp.lib")
#include <iostream>
#include <Windows.h>
#include <winioctl.h>
#include <Dbghelp.h>
#include "Common/Cpp/Unicode.h"
#include "Common/Cpp/PrettyPrint.h"
#include "ErrorReports.h"
#include "ProgramDumper.h"

namespace PokemonAutomation{


class HandleHolder{
public:
    HandleHolder(const HandleHolder&) = delete;
    void operator=(const HandleHolder&) = delete;
    ~HandleHolder(){
        CloseHandle(m_handle);
    }
    HandleHolder(HANDLE handle)
        : m_handle(handle)
    {}
    operator bool() const{
        return m_handle != INVALID_HANDLE_VALUE;
    }
    operator HANDLE() const{
        return m_handle;
    }

private:
    HANDLE m_handle;
};


bool program_dump(Logger* logger, const std::string& filename, EXCEPTION_POINTERS* e){
    HandleHolder handle = CreateFileW(
        utf8_to_wstr(filename).c_str(),
        FILE_WRITE_ACCESS,
        FILE_SHARE_READ,
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        0
    );
    if (!handle){
        DWORD error = GetLastError();
        if (logger){
            logger->log("Unable to create dump file: " + std::to_string(error), COLOR_RED);
        }else{
            std::cout << "Unable to create dump file: " << error << std::endl;
        }
        return false;
    }

    int ret;
    if (e != nullptr){
        MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
        exceptionInfo.ThreadId = GetCurrentThreadId();
        exceptionInfo.ExceptionPointers = e;
        exceptionInfo.ClientPointers = FALSE;
        ret = MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            handle,
            MiniDumpNormal,
            &exceptionInfo,
            nullptr,
            nullptr
        );
    }else{
        ret = MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            handle,
            MiniDumpNormal,
            nullptr,
            nullptr,
            nullptr
        );
    }
    if (!ret){
        DWORD error = GetLastError();
        if (logger){
            logger->log("Unable to create dump file: " + std::to_string(error), COLOR_RED);
        }else{
            std::cout << "Unable to create dump file: " << error << std::endl;
        }
        return false;
    }

    return true;
}
bool program_dump(Logger* logger, const std::string& filename){
    return program_dump(logger, filename, nullptr);
}



long WINAPI crash_handler(EXCEPTION_POINTERS* e){
    static bool handled = false;
    if (handled){
        return EXCEPTION_CONTINUE_SEARCH;
    }
    handled = true;

    SendableErrorReport report;

//    _wmkdir(utf8_to_wstr(ERROR_PATH_UNSENT).c_str());
    program_dump(nullptr, report.directory() + ERROR_DUMP_NAME, e);
    report.save(nullptr);

    Sleep(1000);

    return EXCEPTION_CONTINUE_SEARCH;
}


void setup_crash_handler(){
//    AddVectoredExceptionHandler(0, crash_handler);
    SetUnhandledExceptionFilter(crash_handler);
}



}
