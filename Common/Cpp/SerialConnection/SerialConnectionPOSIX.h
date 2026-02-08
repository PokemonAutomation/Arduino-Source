/*  Serial Connection for POSIX
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_SerialConnectionPOSIX_H
#define PokemonAutomation_SerialConnectionPOSIX_H

#include <string>
#include <atomic>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PanicDump.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/ThreadPool.h"
#include "Common/Cpp/StreamConnections/StreamConnection.h"

namespace PokemonAutomation{

void serial_debug_log(const std::string& msg);



class SerialConnection : public StreamConnection{
public:
    //  UTF-8
    SerialConnection(
        ThreadPool& thread_pool,
        const std::string& name,
        uint32_t baud_rate
    )
        : m_exit(false)
    {
        speed_t baud = B9600;
        switch (baud_rate){
        case 9600:   baud = B9600;   break;
        case 19200:  baud = B19200;  break;
        case 38400:  baud = B38400;  break;
        case 57600:  baud = B57600;  break;
        case 115200: baud = B115200; break;
        default:
            throw ConnectionException(nullptr, "Unsupported Baud Rate: " + std::to_string(baud_rate));
        }
//        std::cout << "desired baud = " << baud << std::endl;

        m_fd = open(name.c_str(), O_RDWR | O_NOCTTY);
        if (m_fd == -1){
            int error = errno;
            std::string str = "Unable to open serial connection. Error = " + std::to_string(error);
            if (error == EACCES){
                str += " (permission denied)\nPlease run as sudo.";
            }
            throw ConnectionException(nullptr, std::move(str));
        }

        struct termios options;
        if (tcgetattr(m_fd, &options) == -1){
            int error = errno;
            throw ConnectionException(nullptr, "tcgetattr() failed. Error = " + std::to_string(error));
        }
//        std::cout << "read baud = " << cfgetispeed(&options) << std::endl;
//        std::cout << "write baud = " << cfgetospeed(&options) << std::endl;

        //  Baud Rate
        if (cfsetispeed(&options, baud) == -1){
            int error = errno;
            throw ConnectionException(nullptr, "cfsetispeed() failed. Error = " + std::to_string(error));
        }
        if (cfsetospeed(&options, baud) == -1){
            int error = errno;
            throw ConnectionException(nullptr, "cfsetospeed() failed. Error = " + std::to_string(error));
        }
//        std::cout << "write baud = " << cfgetispeed(&options) << std::endl;
//        std::cout << "write baud = " << cfgetospeed(&options) << std::endl;

#if 0
        std::cout << "BRKINT  = " << (options.c_iflag & BRKINT) << std::endl;
        std::cout << "ICRNL   = " << (options.c_iflag & ICRNL) << std::endl;
        std::cout << "IMAXBEL = " << (options.c_iflag & IMAXBEL) << std::endl;
        std::cout << "OPOST   = " << (options.c_oflag & OPOST) << std::endl;
        std::cout << "ONLCR   = " << (options.c_oflag & ONLCR) << std::endl;
        std::cout << "ISIG    = " << (options.c_lflag & ISIG) << std::endl;
        std::cout << "ICANON  = " << (options.c_lflag & ICANON) << std::endl;
        std::cout << "ECHO    = " << (options.c_lflag & ECHO) << std::endl;
        std::cout << "ECHOE   = " << (options.c_lflag & ECHOE) << std::endl;
#endif
        //  Configure for raw binary mode (8 bits, no parity, 1 stop bit)
        // No parity
        options.c_cflag &= ~PARENB;
        // 1 stop bit
        options.c_cflag &= ~CSTOPB;
        // Clear size bits
        options.c_cflag &= ~CSIZE;
        // 8 data bits
        options.c_cflag |= CS8;
        // Ignore modem control lines. Important for USB-to-serial adapters (Arduino, etc.) that don't have real modem signals.
        // Without this, the port may wait for carrier detect.
        options.c_cflag |= CLOCAL;
        // Enable receiver. Should be on by default, but some systems don't enable it automatically.
        options.c_cflag |= CREAD;

        //  Disable all input processing for raw binary mode.
        //  This prevents POSIX from treating the data as text and mangling it.

        // Don't send SIGINT on break
        options.c_iflag &= ~BRKINT;
        // Don't strip 8th bit. Critical for binary data 
        options.c_iflag &= ~ISTRIP;
        // Don't map CR to NL
        options.c_iflag &= ~ICRNL;
        // Disable XON/XOFF flow control (input & output)
        options.c_iflag &= ~(IXON | IXOFF);
        // Don't ring bell on full input buffer
        options.c_iflag &= ~IMAXBEL;

        // Disable output processing
        options.c_oflag &= ~OPOST;
        // Don't map NL to CR-NL
        options.c_oflag &= ~ONLCR;

        // Disable all local processing for raw binary mode

        // Don't generate signals
        options.c_lflag &= ~ISIG;
        // Disable canonical (line-based) mode
        options.c_lflag &= ~ICANON;
        // Don't echo input
        options.c_lflag &= ~ECHO;
        // Don't erase character echo
        options.c_lflag &= ~ECHOE;
        // Disable extended input processing
        options.c_lflag &= ~IEXTEN;

        //  Set blocking read with timeout so read() waits for data but can exit periodically
        //  to check m_exit flag. VMIN=0 with VTIME>0 means: wait up to VTIME for data,
        //  return 0 if timeout, or return immediately if any data arrives.
        // Minimum characters to read (0 = return after timeout)
        options.c_cc[VMIN] = 0;
        // Read timeout in deciseconds (1 = 100ms)
        options.c_cc[VTIME] = 1;

        if (tcsetattr(m_fd, TCSANOW, &options) == -1){
            int error = errno;
            throw ConnectionException(nullptr, "tcsetattr() failed. Error = " + std::to_string(error));
        }

        if (tcgetattr(m_fd, &options) == -1){
            int error = errno;
            throw ConnectionException(nullptr, "tcgetattr() failed. Error = " + std::to_string(error));
        }
        if (cfgetispeed(&options) != baud){
//            std::cout << "actual baud = " << cfgetispeed(&options) << std::endl;
            throw ConnectionException(nullptr, "Unable to set input baud rate.");
        }
        if (cfgetospeed(&options) != baud){
//            std::cout << "actual baud = " << cfgetospeed(&options) << std::endl;
            throw ConnectionException(nullptr, "Unable to set output baud rate.");
        }

        //  Start receiver thread.
        try{
            m_listener = thread_pool.blocking_dispatch([this]{
                run_with_catch(
                    "SerialConnection::SerialConnection()",
                    [this]{ recv_loop(); }
                );
            });
        }catch (...){
            close(m_fd);
            throw;
        }
    }

    virtual ~SerialConnection(){
        if (!m_exit.load(std::memory_order_acquire)){
            stop();
        }
    }

    virtual void stop() noexcept final{
        m_exit.store(true, std::memory_order_release);
        close(m_fd);
        m_listener.wait_and_ignore_exceptions();
    }


private:
    // Send data, retrying until all bytes are sent or connection is closed.
    // If write() returns 0 or negative, retry until success.
    // Send the specified data to the serial port. Returns the # of bytes actually sent.
    // This function is blocking and will only return when one of the following happens:
    // 1. The data is fully sent out. (in which it will return bytes)
    // 2. There is an error. (returns less than bytes)
    // 3. The SerialConnection instance is getting destructed from a different thread.
    // If consecutive connection errors reache 100, throw ConnectionException.
    virtual size_t send(const void* data, size_t bytes){
        WriteSpinLock lg(m_send_lock, "SerialConnection::send()");

        const char* ptr = (const char*)data;
        size_t remaining = bytes;

        while (remaining > 0 && !m_exit.load(std::memory_order_acquire)){
            ssize_t sent = write(m_fd, ptr, remaining);

            if (sent > 0){
                // Successfully wrote some bytes
                ptr += sent;
                remaining -= sent;
            } else {
                // sent <= 0: either EAGAIN/EWOULDBLOCK or error
                int error = errno;
                if (error == EAGAIN || error == EWOULDBLOCK){
                    // Device not ready, sleep briefly and retry
                    usleep(1000);  // 1ms
                } else {
                    // Real error occurred
                    process_error(
                        "Failed to write: " + std::to_string(bytes - remaining) +
                        " / " + std::to_string(bytes) +
                        ", error = " + std::to_string(error),
                        true
                    );
                    return bytes - remaining;
                }
            }
        }

        if (remaining == 0){
            m_consecutive_errors.store(0, std::memory_order_release);
        }

        return bytes - remaining;
    }

    // Dedicated receive thread that loops to block waiting for data. When data is 
    // received on the serial port, the loop immidiately calls `on_recv` to notify
    // listeners attached to the class to process data.
    // This thread ends when the class is destructed.
    //
    // Details:
    // read() blocks up to 100ms (VTIME=1) waiting for data to arrive.
    // If data arrives, calls `on_recv()` to process data and loops to check m_eixt.
    // If no data after timeout, it returns 0 and loops to check m_exit.
    // This loop runs indefinitely until m_exit is set.
    void recv_loop(){
        char buffer[32];
        while (!m_exit.load(std::memory_order_acquire)){
            ssize_t actual = read(m_fd, buffer, sizeof(buffer));
            if (actual > 0){
                m_consecutive_errors.store(0, std::memory_order_release);
                on_recv(buffer, actual);
            } else if (actual < 0){
                // Read error occurred
                int error = errno;
                process_error("read serial POSIX() failed. Error = " + std::to_string(error), false);
                usleep(1000);
            }
            // actual == 0: timeout, no data received - just loop again to check m_exit
        }
    }
    

private:
    void process_error(const std::string& message, bool allow_throw){
        WriteSpinLock lg(m_error_lock);

        const size_t consecutive_errors = m_consecutive_errors.fetch_add(1);

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


private:
    int m_fd;
    std::atomic<bool> m_exit;
    std::atomic<size_t> m_consecutive_errors;
    SpinLock m_send_lock;
    SpinLock m_error_lock;
    AsyncTask m_listener;
};


}

#endif
