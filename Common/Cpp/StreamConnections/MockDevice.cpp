/*  Mock Device
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string.h>
#include "Common/Cpp/Exceptions.h"
#include "Common/PABotBase2/ReliableConnectionLayer/PABotBase2_ConnectionDebug.h"
#include "MockDevice.h"

#include <iostream>
using std::cout;
using std::endl;

#if 1
#define PABB2_DROP_HOST_TO_DEVICE   0.01
#define PABB2_DROP_DEVICE_TO_HOST   0.01
#else
#define PABB2_DROP_HOST_TO_DEVICE   0
#define PABB2_DROP_DEVICE_TO_HOST   0
#endif

namespace PokemonAutomation{

//extern std::mutex print_lock;



MockDevice::MockDevice(ThreadPool& thread_pool)
    : m_device_side_connection(*this)
    , m_host_side_connection(*this)
    , m_connection(m_device_side_connection)
    , m_stopping(false)
{
    m_device_thread = thread_pool.dispatch_now_blocking([this]{ device_thread(); });
    m_host_thread = thread_pool.dispatch_now_blocking([this]{ host_recv_thread(); });
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
    m_device_thread.wait_and_ignore_exceptions();
    m_host_thread.wait_and_ignore_exceptions();
}
void MockDevice::print() const{
    std::lock_guard<Mutex> lg(m_device_lock);
    std::lock_guard<Mutex> lg1(m_print_lock);
    m_connection.stream_coalescer().print(true);
}


size_t MockDevice::DeviceSideConnection::unreliable_send(const void* data, size_t bytes) noexcept{
    MockDevice& parent = m_parent;
    {
        WriteSpinLock lg(parent.m_device_to_host_lock);
//        cout << "MockDevice::device_send_serial()" << endl;
        if (parent.m_device_to_host_line.size() >= parent.m_device_to_host_capacity){
            return 0;
        }

        if ((rand() % 100) / 100. < PABB2_DROP_DEVICE_TO_HOST){
            std::lock_guard<Mutex> lg1(parent.m_print_lock);
            cout << "**Intentionally Dropping Packet: device -> host**" << endl;
            return 0;
        }

        bytes = std::min(bytes, parent.m_device_to_host_capacity - parent.m_device_to_host_line.size());
        parent.m_device_to_host_line.insert(
            parent.m_device_to_host_line.end(),
            (const uint8_t*)data,
            (const uint8_t*)data + bytes
        );

        if ((rand() % 100) / 100. < PABB2_DROP_DEVICE_TO_HOST){
            std::lock_guard<Mutex> lg1(parent.m_print_lock);
            cout << "**Intentionally Corrupting Packet: device -> host**" << endl;
            parent.m_device_to_host_line[rand() % parent.m_device_to_host_line.size()] = 0;
        }
    }
    {
        std::lock_guard<Mutex> lg(parent.m_host_lock);
    }
    parent.m_host_cv.notify_all();
    return bytes;
}
size_t MockDevice::DeviceSideConnection::unreliable_recv(void* data, size_t max_bytes, const WallDuration& timeout) noexcept{
    MockDevice& parent = m_parent;
    WriteSpinLock lg(parent.m_host_to_device_lock);
    size_t bytes = std::min(max_bytes, parent.m_host_to_device_line.size());
    auto iter0 = parent.m_host_to_device_line.begin();
    auto iter1 = iter0 + bytes;
    std::copy(iter0, iter1, (uint8_t*)data);
    parent.m_host_to_device_line.erase(iter0, iter1);

//    {
//        std::lock_guard<Mutex> lg1(PokemonAutomation::print_lock);
//        cout << "device_read_serial(): attempt = " << max_bytes << ", actual = " << bytes << endl;
//    }

//    if (bytes > 0){
//        PacketHeader_check((const PABotBase2::PacketHeader*)data, true);
//    }

    return bytes;
}


size_t MockDevice::HostSideConnection::unreliable_send(const void* data, size_t bytes) noexcept{
    MockDevice& parent = m_parent;
    {
        WriteSpinLock lg(parent.m_host_to_device_lock);
//        cout << "MockDevice::send(const void* data, size_t bytes)" << endl;

#if 0
        if (((const PABotBase2::PacketHeader*)data)->magic_number != 0x81){
            cout << "corrupted packet!" << endl;
            PABotBase2::PacketHeader_print((const PABotBase2::PacketHeader*)data, true);
        }
        PABotBase2::PacketHeader_check((const PABotBase2::PacketHeader*)data, true);
#endif

        if ((rand() % 100) / 100. < PABB2_DROP_HOST_TO_DEVICE){
            std::lock_guard<Mutex> lg1(parent.m_print_lock);
            cout << "**Intentionally Dropping Packet: host -> device**" << endl;
            return 0;
        }

        if (parent.m_host_to_device_line.size() >= parent.m_host_to_device_capacity){
            return 0;
        }

//        bytes = std::min(bytes, m_host_to_device_capacity - m_host_to_device_line.size());
//        cout << "MockDevice::send(): " << bytes << endl;

        parent.m_host_to_device_line.insert(
            parent.m_host_to_device_line.end(),
            (const uint8_t*)data,
            (const uint8_t*)data + bytes
        );

        if ((rand() % 100) / 100. < PABB2_DROP_HOST_TO_DEVICE){
            std::lock_guard<Mutex> lg1(parent.m_print_lock);
            cout << "**Intentionally Corrupting Packet: host -> device**" << endl;
            parent.m_host_to_device_line[rand() % parent.m_host_to_device_line.size()] = 0;
        }
    }
    {
        std::lock_guard<Mutex> lg(parent.m_device_lock);
//        cout << "MockDevice::send(const void* data, size_t bytes) - notifying" << endl;
    }
    parent.m_device_cv.notify_all();
    return bytes;
}

void MockDevice::push_expected_stream_data(const void* data, size_t bytes){
    {
        std::lock_guard<Mutex> lg(m_device_lock);

        m_expected_host_to_device_stream.insert(
            m_expected_host_to_device_stream.end(),
            (const uint8_t*)data,
            (const uint8_t*)data + bytes
        );
    }
    verify_stream_data();
}
size_t MockDevice::verify_stream_data(){
    std::lock_guard<Mutex> lg(m_device_lock);
    size_t bytes = m_expected_host_to_device_stream.size();

    std::vector<uint8_t> actual(bytes);
    size_t read = m_connection.reliable_recv(actual.data(), bytes);

    if (read == 0){
        return bytes;
    }


#if 0
    {
        std::lock_guard<Mutex> lg1(m_print_lock);
        cout << "read = " << read << endl;
    }
#endif


    std::vector<uint8_t> expected(
        m_expected_host_to_device_stream.begin(),
        m_expected_host_to_device_stream.begin() + read
    );

    m_expected_host_to_device_stream.erase(
        m_expected_host_to_device_stream.begin(),
        m_expected_host_to_device_stream.begin() + read
    );

    bool matched = memcmp(actual.data(), expected.data(), read) == 0;

    std::lock_guard<Mutex> lg1(m_print_lock);
    if (matched){
#if 0
        cout << "Matched: Bytes = " << read
             << ", Remaining = " << m_expected_host_to_device_stream.size()
             << ", Matched = "
             << std::string((const char*)actual.data(), read)
             << endl;
#endif
    }else{
        cout << "MISMATCH: Expected = "
             << std::string((const char*)expected.data(), read)
             << ", Actual = "
             << std::string((const char*)actual.data(), read)
             << endl;
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Mismatch");
    }
    return m_expected_host_to_device_stream.size();
}




void MockDevice::device_thread(){
    std::unique_lock<Mutex> lg(m_device_lock);
    while (!m_stopping.load(std::memory_order_relaxed)){
        m_connection.run_send_events(Milliseconds(0));
        m_connection.run_recv_events(Milliseconds(0));
        m_device_cv.wait_for(lg, Milliseconds(10));
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
        m_host_side_connection.on_unreliable_recv(data.data(), data.size());
    }
}
















}
