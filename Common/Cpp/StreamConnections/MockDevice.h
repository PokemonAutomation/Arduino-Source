/*  Mock Device
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_MockDevice_H
#define PokemonAutomation_MockDevice_H

#include <deque>
#include <mutex>
#include <condition_variable>
#include "Common/Cpp/StreamConnections/StreamConnection.h"
#include "Common/Cpp/Concurrency/Thread.h"
#include "Common/PABotBase2/PABotbase2_ReliableStreamConnection.h"

namespace PokemonAutomation{




class MockDevice : public StreamConnection{
public:
    MockDevice();
    ~MockDevice();


private:
    //  Call from device.

    static size_t fp_device_send_serial(
        void* context,
        const void* data, size_t bytes,
        bool is_retransmit
    ){
        return ((MockDevice*)context)->device_send_serial(data, bytes, is_retransmit);
    }
    static size_t fp_device_read_serial(
        void* context,
        void* data, size_t max_bytes
    ){
        return ((MockDevice*)context)->device_read_serial(data, max_bytes);
    }

    size_t device_send_serial(const void* data, size_t bytes, bool is_retransmit);
    size_t device_read_serial(void* data, size_t max_bytes);


public:
    //  Call from host.

    virtual size_t send(const void* data, size_t bytes) override;


private:
    void device_thread();
    void host_recv_thread();


private:
    pabb2_ReliableStreamConnection m_connection;

    SpinLock m_device_to_host_lock;
    size_t m_device_to_host_capacity = 1024;
    std::deque<uint8_t> m_device_to_host_line;

    SpinLock m_host_to_device_lock;
    size_t m_host_to_device_capacity = 1024;
    std::deque<uint8_t> m_host_to_device_line;

    std::atomic<bool> m_stopping;

    std::mutex m_device_lock;
    std::condition_variable m_device_cv;
    Thread m_device_thread;

    std::mutex m_host_lock;
    std::condition_variable m_host_cv;
    Thread m_host_thread;
};



}
#endif
