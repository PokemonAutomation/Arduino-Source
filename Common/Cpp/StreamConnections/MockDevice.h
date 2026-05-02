/*  Mock Device
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_MockDevice_H
#define PokemonAutomation_MockDevice_H

#include <deque>
#include "Common/Cpp/StreamConnections/PollingStreamConnections.h"
#include "Common/Cpp/StreamConnections/PushingStreamConnections.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/ThreadPool.h"
#include "Common/PABotBase2/ReliableConnectionLayer/PABotBase2FW_ReliableStreamConnection.h"

namespace PokemonAutomation{




class MockDevice{
public:
    MockDevice(ThreadPool& thread_pool);
    ~MockDevice();

    void print() const;

    Mutex& print_lock() const{
        return m_print_lock;
    }

    UnreliableStreamConnectionPolling& device_side_connection(){
        return m_device_side_connection;
    }

    UnreliableStreamConnectionPushing& host_side_connection(){
        return m_host_side_connection;
    }


public:
    //  Call from host.

    void push_expected_stream_data(const void* data, size_t bytes);
    size_t verify_stream_data();


private:
    void device_thread();
    void host_recv_thread();


private:
    class DeviceSideConnection : public UnreliableStreamConnectionPolling{
    public:
        DeviceSideConnection(MockDevice& parent) : m_parent(parent) {}
        virtual size_t unreliable_send(const void* data, size_t bytes) noexcept override;
        virtual size_t unreliable_recv(void* data, size_t max_bytes, const WallDuration& timeout) noexcept override;
    private:
        MockDevice& m_parent;
    };

    class HostSideConnection : public UnreliableStreamConnectionPushing{
    public:
        HostSideConnection(MockDevice& parent) : m_parent(parent) {}
        virtual size_t unreliable_send(const void* data, size_t bytes) noexcept override;
        using UnreliableStreamConnectionPushing::on_unreliable_recv;
    private:
        MockDevice& m_parent;
    };

    DeviceSideConnection m_device_side_connection;
    HostSideConnection m_host_side_connection;

    PABotBase2::ReliableStreamConnectionFW m_connection;

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
