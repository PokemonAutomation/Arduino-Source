/*  Mock Device
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_MockDevice_H
#define PokemonAutomation_MockDevice_H

#include <deque>
#include "Common/Cpp/StreamConnections/StreamConnection.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/ThreadPool.h"
#include "Common/PABotBase2/PABotbase2_ReliableStreamConnection.h"

namespace PokemonAutomation{




class MockDevice : public StreamConnection{
public:
    MockDevice(ThreadPool& thread_pool);
    ~MockDevice();

    void print() const;

    Mutex& print_lock() const{
        return m_print_lock;
    }


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

    void push_expected_stream_data(const void* data, size_t bytes);
    size_t verify_stream_data();


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

    std::deque<uint8_t> m_expected_host_to_device_stream;

    std::atomic<bool> m_stopping;

    mutable Mutex m_device_lock;
    ConditionVariable m_device_cv;
    AsyncTask m_device_thread;

    mutable Mutex m_host_lock;
    ConditionVariable m_host_cv;
    AsyncTask m_host_thread;

    mutable Mutex m_print_lock;
};



}
#endif
