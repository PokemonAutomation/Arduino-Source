/*  Mock Device
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "MockDevice.h"

namespace PokemonAutomation{


MockDevice::MockDevice()
    : m_stopping(false)
{
    pabb2_ReliableStreamConnection_init(
        &m_connection,
        this,
        fp_device_send_serial,
        fp_device_read_serial
    );

    m_device_thread = Thread([this]{ device_thread(); });
}
MockDevice::~MockDevice(){
    m_stopping.store(true, std::memory_order_release);
    {
        std::lock_guard<std::mutex> lg(m_device_lock);
    }
    {
        std::lock_guard<std::mutex> lg(m_host_lock);
    }
    m_device_cv.notify_all();
    m_host_cv.notify_all();
    m_device_thread.join();
    m_host_thread.join();
}


size_t MockDevice::device_send_serial(const void* data, size_t bytes){
    WriteSpinLock lg(m_device_to_host_lock);
    if (m_device_to_host_line.size() >= m_device_to_host_capacity){
        return 0;
    }
    bytes = std::min(bytes, m_device_to_host_capacity - m_device_to_host_line.size());
    m_device_to_host_line.insert(
        m_device_to_host_line.end(),
        (const uint8_t*)data,
        (const uint8_t*)data + bytes
    );
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
    WriteSpinLock lg(m_host_to_device_lock);
    if (m_host_to_device_line.size() >= m_host_to_device_capacity){
        return 0;
    }
    bytes = std::min(bytes, m_host_to_device_capacity - m_host_to_device_line.size());
    m_host_to_device_line.insert(
        m_host_to_device_line.end(),
        (const uint8_t*)data,
        (const uint8_t*)data + bytes
    );
    return bytes;
}


void MockDevice::device_thread(){
    std::unique_lock<std::mutex> lg(m_device_lock);
    while (!m_stopping.load(std::memory_order_relaxed)){
        m_device_cv.wait(lg);
        pabb2_ReliableStreamConnection_run_events(&m_connection);
    }
}
void MockDevice::host_recv_thread(){
    while (!m_stopping.load(std::memory_order_relaxed)){
        std::vector<uint8_t> data;
        {
            std::unique_lock<std::mutex> lg0(m_host_lock);
            {
                WriteSpinLock lg1(m_device_to_host_lock);
                data = std::vector<uint8_t>(m_device_to_host_line.begin(), m_device_to_host_line.end());
                m_device_to_host_line.clear();
            }
            if (data.empty()){
                m_host_cv.wait(lg0);
                continue;
            }
        }
        on_recv(data.data(), data.size());
    }
}
















}
