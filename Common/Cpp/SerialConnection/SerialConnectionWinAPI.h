/*  Serial Connection for Windows
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_SerialConnectionWinAPI_H
#define PokemonAutomation_SerialConnectionWinAPI_H

#include <string>
#include <atomic>
#include <windows.h>
#include "Common/Compiler.h"
#include "Common/Cpp/Time.h"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PanicDump.h"
#include "Common/Cpp/Strings/Unicode.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Concurrency/Thread.h"
#include "Common/Cpp/StreamConnections/StreamConnection.h"

namespace PokemonAutomation{

void serial_debug_log(const std::string& msg);



class SerialConnection : public StreamConnection{
public:
    //  UTF-8
    SerialConnection(const std::string& name, uint32_t baud_rate)
        : SerialConnection(name, utf8_to_wstr(name), baud_rate)
    {}
    SerialConnection(const std::string& name, const std::wstring& wname, uint32_t baud_rate)
        : m_exit(false)
        , m_consecutive_errors(0)
    {
        m_handle = CreateFileW(
            (L"\\\\.\\" + wname).c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0, 0,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            0
        );
        if (m_handle == INVALID_HANDLE_VALUE){
            DWORD error = GetLastError();
            throw ConnectionException(nullptr, "Unable to open serial connection (" + name + "). Error = " + std::to_string(error));
        }

        DCB serial_params{};
        serial_params.DCBlength = sizeof(serial_params);

        if (!GetCommState(m_handle, &serial_params)){
            DWORD error = GetLastError();
            CloseHandle(m_handle);
            throw ConnectionException(nullptr, "GetCommState() failed. Error = " + std::to_string(error));
        }
//        cout << "BaudRate = " << (int)serial_params.BaudRate << endl;
//        cout << "ByteSize = " << (int)serial_params.ByteSize << endl;
//        cout << "StopBits = " << (int)serial_params.StopBits << "0 means 1 bit" << endl;
//        cout << "Parity   = " << (int)serial_params.Parity << endl;
        serial_params.BaudRate = baud_rate;
        serial_params.ByteSize = 8;
        serial_params.StopBits = 0;
        serial_params.Parity = 0;
        if (!SetCommState(m_handle, &serial_params)){
            DWORD error = GetLastError();
            CloseHandle(m_handle);
            throw ConnectionException(nullptr, "SetCommState() failed. Error = " + std::to_string(error));
        }

#if 1
        COMMTIMEOUTS timeouts{};
        if (!GetCommTimeouts(m_handle, &timeouts)){
            DWORD error = GetLastError();
            CloseHandle(m_handle);
            throw ConnectionException(nullptr, "GetCommTimeouts() failed. Error = " + std::to_string(error));
        }

        //std::cout << "ReadIntervalTimeout = " << timeouts.ReadIntervalTimeout << std::endl;
        //std::cout << "ReadTotalTimeoutMultiplier = " << timeouts.ReadTotalTimeoutMultiplier << std::endl;
        //std::cout << "ReadTotalTimeoutConstant = " << timeouts.ReadTotalTimeoutConstant << std::endl;
        //std::cout << "WriteTotalTimeoutMultiplier = " << timeouts.WriteTotalTimeoutMultiplier << std::endl;
        //std::cout << "WriteTotalTimeoutConstant = " << timeouts.WriteTotalTimeoutConstant << std::endl;

#if 1
        timeouts = COMMTIMEOUTS{(DWORD)-1, 0, 0, 0, 100};
#else
        //  Need to set a read timer. In some cases, a pending ReadFile() call
        //  will block a WriteFile() call until it returns - leading to a
        //  deadlock if the device isn't sending any information.
        timeouts.ReadIntervalTimeout = 0;
        timeouts.ReadTotalTimeoutMultiplier = 0;
        timeouts.ReadTotalTimeoutConstant = 1;
#endif
        if (!SetCommTimeouts(m_handle, &timeouts)){
            DWORD error = GetLastError();
            CloseHandle(m_handle);
            throw ConnectionException(nullptr, "SetCommTimeouts() failed. Error = " + std::to_string(error));
        }
#endif

        //  Start receiver thread.
        try{
            m_listener = Thread([this]{
                run_with_catch(
                    "SerialConnection::SerialConnection()",
                    [this]{ recv_loop(); }
                );
            });
        }catch (...){
            CloseHandle(m_handle);
            throw;
        }
    }
    virtual ~SerialConnection(){
        if (!m_exit.load(std::memory_order_acquire)){
            stop();
        }
    }

    virtual void stop() final{
        m_exit.store(true, std::memory_order_release);
        CloseHandle(m_handle);
        m_listener.join();
    }

private:
    void process_error(const std::string& message, bool allow_throw){
        WriteSpinLock lg(m_error_lock);

        size_t consecutive_errors = m_consecutive_errors.fetch_add(1);

        serial_debug_log(message);

        std::string clear_error;
        DWORD comm_error;
        if (ClearCommError(m_handle, &comm_error, nullptr) == 0){
            DWORD error = GetLastError();
            clear_error = "ClearCommError() failed. Error = " + std::to_string(error);
        }else{
            clear_error = "ClearCommError error flag = " + std::to_string(comm_error);
        }

        if (consecutive_errors <= 10){
            serial_debug_log(message);
        }
        if (consecutive_errors == 10){
            serial_debug_log("Further error messages will be suppressed.");
        }
        if (consecutive_errors >= 100 && allow_throw){
            throw ConnectionException(nullptr, "Serial Connection failed.");
        }
    }


    virtual size_t send(const void* data, size_t bytes){
        WriteSpinLock lg(m_send_lock, "SerialConnection::send()");
#if 0
        for (size_t c = 0; c < bytes; c++){
            std::cout << "Send: " << (int)((const char*)data)[c] << std::endl;
        }
#endif

//        std::cout << "start write" << std::endl;
//        auto start = current_time();
        DWORD written;
        if (WriteFile(m_handle, data, (DWORD)bytes, &written, nullptr) != 0 && bytes == written){
            m_consecutive_errors.store(0, std::memory_order_release);
            return written;
        }

        DWORD error = GetLastError();
        process_error(
            "Failed to write: " + std::to_string(written) +
            " / " + std::to_string(bytes) +
            ", error = " + std::to_string(error),
            true
        );
//        auto stop = current_time();
//        cout << "WriteFile() : " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << endl;

//        std::cout << "end send()" << std::endl;

        return written;
    }

    void recv_loop(){
//        std::lock_guard<Mutex> lg(m_send_lock);
        char buffer[32];
        auto last_recv = current_time();
        while (!m_exit.load(std::memory_order_acquire)){
//            auto start = current_time();
//            std::cout << "start read" << std::endl;
            DWORD read;
            if (ReadFile(m_handle, buffer, 32, &read, nullptr) == 0){
                DWORD error = GetLastError();
                process_error("ReadFile() failed. Error = " + std::to_string(error), false);
            }
//            auto stop = current_time();
//            cout << "ReadFile() : " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << endl;
//            std::cout << "read = " << read << std::endl;
//            set_timouts();
#if 0
            for (size_t c = 0; c < read; c++){
                cout << "Recv: " << (int)buffer[c] << endl;
            }
#endif
            if (read != 0){
                m_consecutive_errors.store(0, std::memory_order_release);
                on_recv(buffer, read);
                last_recv = current_time();
                continue;
            }


#if 1
//            auto start = current_time();
            Sleep(1);
//            auto stop = current_time();
#else
            auto now = current_time();
            uint64_t millis_since_last_recv = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_recv).count();
            if (millis_since_last_recv > 100){
                Sleep(1);
            }else{
                pause();
            }
#endif
        }
    }

private:
    HANDLE m_handle;
    std::atomic<bool> m_exit;
    std::atomic<size_t> m_consecutive_errors;
    SpinLock m_send_lock;
    SpinLock m_error_lock;
    Thread m_listener;
};



}

#endif
