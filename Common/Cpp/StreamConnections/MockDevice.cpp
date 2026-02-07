/*  Mock Device
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/PABotBase2/PABotBase2_ConnectionDebug.h"

#include "MockDevice.h"

//  REMOVE
#include <iostream>
using std::cout;
using std::endl;

#if 0
#define PABB2_DROP_HOST_TO_DEVICE   0.2
#define PABB2_DROP_DEVICE_TO_HOST   0.2
#else
#define PABB2_DROP_HOST_TO_DEVICE   0
#define PABB2_DROP_DEVICE_TO_HOST   0
#endif

namespace PokemonAutomation{


MockDevice::MockDevice(ThreadPool& thread_pool)
    : m_stopping(false)
{
    pabb2_ReliableStreamConnection_init(
        &m_connection,
        this,
        fp_device_send_serial,
        fp_device_read_serial
    );

    m_device_thread = thread_pool.blocking_dispatch([this]{ device_thread(); });
    m_host_thread = thread_pool.blocking_dispatch([this]{ host_recv_thread(); });
}
MockDevice::~MockDevice(){
    m_stopping.store(true, std::memory_order_release);
    {
        std::lock_guard<Mutex> lg(m_device_lock);
    }
    {
        std::lock_guard<Mutex> lg(m_host_lock);
    }
    m_device_cv.notify_all();
    m_host_cv.notify_all();
    m_device_thread.reset();
    m_host_thread.reset();
}
void MockDevice::print() const{
    std::lock_guard<Mutex> lg(m_device_lock);
    pabb2_StreamCoalescer_print(&m_connection.stream_coalescer, true);
}


size_t MockDevice::device_send_serial(const void* data, size_t bytes, bool is_retransmit){
    {
        WriteSpinLock lg(m_device_to_host_lock);
//        cout << "MockDevice::device_send_serial()" << endl;
        if (m_device_to_host_line.size() >= m_device_to_host_capacity){
            return 0;
        }

        if ((rand() % 100) / 100. < PABB2_DROP_DEVICE_TO_HOST){
            cout << "**Intentionally Dropping Packet: device -> host**" << endl;
            return 0;
        }

        bytes = std::min(bytes, m_device_to_host_capacity - m_device_to_host_line.size());
        m_device_to_host_line.insert(
            m_device_to_host_line.end(),
            (const uint8_t*)data,
            (const uint8_t*)data + bytes
        );

        if ((rand() % 100) / 100. < PABB2_DROP_DEVICE_TO_HOST){
            cout << "**Intentionally Corrupting Packet: device -> host**" << endl;
            m_device_to_host_line[rand() % m_device_to_host_line.size()] = 0;
        }
    }
    {
        std::lock_guard<Mutex> lg(m_host_lock);
    }
    m_host_cv.notify_all();
    return bytes;
}
size_t MockDevice::device_read_serial(void* data, size_t max_bytes){
    WriteSpinLock lg(m_host_to_device_lock);
    size_t bytes = std::min(max_bytes, m_host_to_device_line.size());
    auto iter0 = m_host_to_device_line.begin();
    auto iter1 = iter0 + bytes;
    std::copy(iter0, iter1, (uint8_t*)data);
    m_host_to_device_line.erase(iter0, iter1);
    return bytes;
}


size_t MockDevice::send(const void* data, size_t bytes){
    {
        WriteSpinLock lg(m_host_to_device_lock);
//        cout << "MockDevice::send(const void* data, size_t bytes)" << endl;

        if ((rand() % 100) / 100. < PABB2_DROP_HOST_TO_DEVICE){
            cout << "**Intentionally Dropping Packet: host -> device**" << endl;
            return 0;
        }

        if (m_host_to_device_line.size() >= m_host_to_device_capacity){
            return 0;
        }
        bytes = std::min(bytes, m_host_to_device_capacity - m_host_to_device_line.size());
        m_host_to_device_line.insert(
            m_host_to_device_line.end(),
            (const uint8_t*)data,
            (const uint8_t*)data + bytes
        );

        if ((rand() % 100) / 100. < PABB2_DROP_HOST_TO_DEVICE){
            cout << "**Intentionally Corrupting Packet: host -> device**" << endl;
            m_host_to_device_line[rand() % m_host_to_device_line.size()] = 0;
        }
    }
    {
        std::lock_guard<Mutex> lg(m_device_lock);
//        cout << "MockDevice::send(const void* data, size_t bytes) - notifying" << endl;
    }
    m_device_cv.notify_all();
    return bytes;
}


void MockDevice::device_thread(){
    std::unique_lock<Mutex> lg(m_device_lock);
    while (!m_stopping.load(std::memory_order_relaxed)){
        pabb2_ReliableStreamConnection_run_events(&m_connection);
        m_device_cv.wait(lg);
    }
}
void MockDevice::host_recv_thread(){
    while (!m_stopping.load(std::memory_order_relaxed)){
        std::vector<uint8_t> data;
        {
            std::unique_lock<Mutex> lg0(m_host_lock);
            {
                WriteSpinLock lg1(m_device_to_host_lock);
                data = std::vector<uint8_t>(m_device_to_host_line.begin(), m_device_to_host_line.end());
                m_device_to_host_line.clear();
            }
//            cout << "data.size() = " << data.size() << endl;
            if (data.empty()){
                m_host_cv.wait(lg0);
                continue;
            }
        }
//        cout << "Passing data to host: " << data.size() << endl;
        on_recv(data.data(), data.size());
    }
}
















}
