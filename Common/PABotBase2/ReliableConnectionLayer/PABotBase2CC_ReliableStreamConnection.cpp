/*  Reliable Stream Connection (CC)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/CRC32/pabb_CRC32.h"
#include "Common/Cpp/PrettyPrint.h"
//#include "Common/Cpp/Exceptions.h"
#include "Common/PABotBase2/PABotBase2CC_MessageDumper.h"
//#include "PABotBase2_ConnectionDebug.h"
#include "PABotBase2CC_ReliableStreamConnection.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace PABotBase2{



ReliableStreamConnection::ReliableStreamConnection(
    CancellableScope* parent,
    Logger& logger, bool log_everything,
    ThreadPool& thread_pool,
    UnreliableStreamConnectionPushing& unreliable_connection,
    WallDuration retransmit_timeout,
    Mutex* print_lock
)
    : m_logger(logger)
    , m_unreliable_connection(unreliable_connection)
    , m_retransmit_timeout(retransmit_timeout)
    , m_print_lock(print_lock)
    , m_reliable_sender(*this, 20)
    , m_log_everything(log_everything)
//    , m_version_verified(false)
    , m_remote_protocol_compatible(false)
    , m_remote_protocol(0)
    , m_remote_slot_capacity(1)
    , m_remote_buffer_capacity(PABB2_PacketSender_BUFFER_SIZE)
{
    m_retransmit_thread = thread_pool.dispatch_now_blocking(
        [this]{ retransmit_thread(); }
    );

    m_unreliable_connection.add_listener(*this);
    if (parent){
        attach(*parent);
    }
}

ReliableStreamConnection::~ReliableStreamConnection(){
    cancel(nullptr);
    detach();
    m_unreliable_connection.remove_listener(*this);
    m_retransmit_thread.wait_and_ignore_exceptions();
}

bool ReliableStreamConnection::cancel(std::exception_ptr exception) noexcept{
    if (Cancellable::cancel(std::move(exception))){
        return true;
    }
    {
        std::lock_guard<Mutex> lg(m_lock);
        m_error = "Connection has been closed.";
    }
    m_cv.notify_all();
    return false;
}
size_t ReliableStreamConnection::pending() const{
    std::unique_lock<Mutex> lg(m_lock);
    return m_reliable_sender.slots_used();
}
bool ReliableStreamConnection::wait_for_pending(WallDuration timeout){
    std::unique_lock<Mutex> lg(m_lock);
    if (timeout == WallDuration::max()){
        m_cv.wait(lg, [this]{
            return this->cancelled() || m_reliable_sender.slots_used() == 0;
        });
    }else{
        m_cv.wait_for(lg, timeout, [this]{
            return this->cancelled() || m_reliable_sender.slots_used() == 0;
        });
    }
    throw_if_cancelled();
    return m_reliable_sender.slots_used() == 0;
}



//
//  StreamSender/StreamListener
//

bool ReliableStreamConnection::reliable_send_all_or_nothing(
    const void* data, size_t bytes,
    WallDuration timeout
) noexcept{
    WallClock deadline = timeout == WallDuration::max()
        ? WallClock::max()
        : current_time() + timeout;

    const char* ptr = (const char*)data;
    std::unique_lock<Mutex> lg(m_lock);
    do{
        throw_if_cancelled();
        if (m_reliable_sender.slots_used() >= m_remote_slot_capacity){
            m_cv.wait_until(lg, deadline);
            continue;
        }
        if (m_reliable_sender.send_stream_all_or_nothing(ptr, bytes)){
            return true;
        }
        m_cv.wait_until(lg, deadline);
    }while (current_time() < deadline);
    return false;
}
void ReliableStreamConnection::on_recv(const void* data, size_t bytes){
#if 0
    if (m_print_lock){
        std::lock_guard<Mutex> lg(*m_print_lock);
        cout << "ReliableStreamConnection::on_recv(): " << bytes << endl;
    }
#endif
    m_parser.push_bytes(*this, (const uint8_t*)data, bytes);
}



//
//  PABotBase2::StreamConnection
//

size_t ReliableStreamConnection::unreliable_send(const void* data, size_t bytes) noexcept{
    const PacketHeader* header = (const PacketHeader*)data;
    uint8_t opcode = header->opcode & PABB2_CONNECTION_OPCODE_MASK;
    bool retransmit = header->opcode & PABB2_CONNECTION_RETRANSMIT_FLAG;

    bool always_log =
        opcode != PABB2_CONNECTION_OPCODE_ASK_STREAM_DATA &&
        opcode != PABB2_CONNECTION_OPCODE_RET_STREAM_DATA;

    try{
        if (retransmit){
            m_logger.log(
                "[RSC]: Re-send: (0x" + tostr_hex(header->opcode) + ") " + tostr(header),
                COLOR_ORANGE
            );
        }else if (m_log_everything || always_log){
            m_logger.log(
                "[RSC]: Sending: (0x" + tostr_hex(header->opcode) + ") " + tostr(header),
                COLOR_DARKGREEN
            );
//            PABotBase2::PacketHeader_print(header, true);
        }
    }catch (...){}
//    cout << "ReliableStreamConnection::unreliable_send() - before send" << endl;
    return m_unreliable_connection.unreliable_send(data, bytes);
}



//
//  Send Path
//

bool ReliableStreamConnection::reset(WallDuration timeout){
    {
        std::lock_guard<Mutex> lg(m_lock);
        m_reliable_sender.reset();
        m_parser.reset();
        m_stream_coalescer.reset();
        throw_if_cancelled();
        m_reliable_sender.send_packet(PABB2_CONNECTION_OPCODE_ASK_RESET, 0, nullptr);
    }
    m_cv.notify_all();
    return wait_for_pending(timeout);
}

bool ReliableStreamConnection::try_send_request(uint8_t opcode){
    std::lock_guard<Mutex> lg(m_lock);
//    cout << "Sending: " << tostr_hex(opcode) << endl;
    throw_if_cancelled();
    if (m_reliable_sender.slots_used() >= m_remote_slot_capacity){
        return 0;
    }
    return m_reliable_sender.send_packet(opcode, 0, nullptr);
}
void ReliableStreamConnection::send_request(uint8_t opcode){
    std::unique_lock<Mutex> lg(m_lock);
    while (true){
        throw_if_cancelled();
        if (m_reliable_sender.slots_used() < m_remote_slot_capacity &&
            m_reliable_sender.send_packet(opcode, 0, nullptr)
        ){
            return;
        }
        m_cv.wait(lg);
    }
}

void ReliableStreamConnection::print() const{
    std::unique_lock<Mutex> lg(m_lock);
    m_reliable_sender.print(true);
//    StreamCoalescer_print(&m_stream_coalescer, true);
}


void ReliableStreamConnection::send_ack(uint8_t seqnum, uint8_t opcode){
    //  Must call inside lock.
    struct{
        PacketHeader header;
        uint8_t crc[sizeof(uint32_t)];
    } packet;
    packet.header.magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
    packet.header.seqnum = seqnum;
    packet.header.packet_bytes = sizeof(packet);
    packet.header.opcode = opcode;
    pabb_crc32_write_to_message(&packet, sizeof(packet));
    unreliable_send(&packet, sizeof(packet));
}
void ReliableStreamConnection::send_ack_u16(uint8_t seqnum, uint8_t opcode, uint16_t data){
    //  Must call inside lock.
    struct{
        PacketHeader_u16 header;
        uint8_t crc[sizeof(uint32_t)];
    } packet;
    packet.header.magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
    packet.header.seqnum = seqnum;
    packet.header.packet_bytes = sizeof(packet);
    packet.header.opcode = opcode;
    packet.header.data = data;
    pabb_crc32_write_to_message(&packet, sizeof(packet));
    unreliable_send(&packet, sizeof(packet));
}

void ReliableStreamConnection::retransmit_thread(){
    WallClock next_retransmit = current_time() + m_retransmit_timeout;
    while (true){
        std::unique_lock<Mutex> lg(m_lock);
        if (this->cancelled()){
            break;
        }

        m_cv.wait_until(lg, next_retransmit);

        if (this->cancelled()){
            break;
        }
        if (m_reliable_sender.slots_used() == 0){
            continue;
        }
        if (current_time() < next_retransmit){
            continue;
        }

//        cout << "running retransmits" << endl;

#if 1
        if (!m_reliable_sender.iterate_retransmits()){
//            cout << "nothing to do" << endl;
            next_retransmit = current_time() + m_retransmit_timeout;
        }else{
//            cout << "did something" << endl;
        }
#else
        next_retransmit = current_time() + m_retransmit_timeout;
#endif
    }
}



//
//  PABotBase2::PacketRunner
//

void ReliableStreamConnection::on_packet(const PacketHeader* packet){
    uint8_t status = packet->magic_number;

//    {
//        std::lock_guard<std::mutex> lg(PokemonAutomation::print_lock);
//        cout << "on_packet(): seqnum = [" << (int)packet->seqnum << "], opcode = " << (int)packet->opcode << endl;
//    }

//    cout << "on_packet(): ";
//    PABotBase2::print_bytes(packet, packet->packet_bytes, false);
//    cout << endl;

    //
    //  Process the packet status.
    //

    switch (status){
    case PABB2_PacketParser_RESULT_VALID:
        break;
    case PABB2_PacketParser_RESULT_INVALID:
        m_logger.log(
            "[RSC]: INVALID PACKET: Received invalid packet from device.",
            COLOR_RED
        );
        return;
    case PABB2_PacketParser_RESULT_CHECKSUM_FAIL:
        m_logger.log(
            "[RSC]: CHECKSUM MISMATCH: Received bad data from device: bytes = " + std::to_string(packet->packet_bytes),
//            "[RSC]: CHECKSUM MISMATCH: Received bad data from device: bytes = " + tostr(packet),
            COLOR_RED
        );
        return;
    default:
        m_logger.log(
            "[RSC]: Internal Error: Packet parser returned invalid code: " + std::to_string(status),
            COLOR_RED
        );
        return;
    }



    //
    //  Process the packet itself.
    //

    if (m_log_everything){
        m_logger.log("[RSC]: Receive: (0x" + tostr_hex(packet->opcode) + ") " + tostr(packet), COLOR_PURPLE);
    }

    uint8_t opcode = packet->opcode & PABB2_CONNECTION_OPCODE_MASK;
    switch (opcode){
    case PABB2_CONNECTION_OPCODE_INVALID_LENGTH:
        m_logger.log(
            "[RSC]: PABB2_CONNECTION_OPCODE_INVALID_LENGTH: Device reported an invalid message length.",
            COLOR_RED
        );
        return;
    case PABB2_CONNECTION_OPCODE_INVALID_CHECKSUM_FAIL:
        m_logger.log(
            "[RSC]: PABB2_CONNECTION_OPCODE_INVALID_CHECKSUM_FAIL: Device reported a checksum mismatch.",
            COLOR_RED
        );
        return;
    case PABB2_CONNECTION_OPCODE_UNKNOWN_OPCODE:
        process_UNKNOWN_OPCODE(packet);
        return;
    case PABB2_CONNECTION_OPCODE_ASK_STREAM_DATA:
        process_ASK_STREAM_DATA(packet);
        return;
    case PABB2_CONNECTION_OPCODE_RET_STREAM_DATA:
        process_RET_STREAM_DATA(packet);
        return;
    case PABB2_CONNECTION_OPCODE_RET_RESET:
        process_RET_RESET(packet);
        return;
    case PABB2_CONNECTION_OPCODE_RET_VERSION:
        process_RET_VERSION(packet);
        return;
    case PABB2_CONNECTION_OPCODE_RET_PACKET_SIZE:
        process_RET_PACKET_SIZE(packet);
        return;
    case PABB2_CONNECTION_OPCODE_RET_BUFFER_SLOTS:
        process_RET_BUFFER_SLOTS(packet);
        return;
    case PABB2_CONNECTION_OPCODE_INFO_STREAM_DEAD:
    case PABB2_CONNECTION_OPCODE_INFO_STREAM_NOT_READY:
    case PABB2_CONNECTION_OPCODE_INFO_STREAM_SEND_FULL:
    case PABB2_CONNECTION_OPCODE_INFO_STREAM_RECV_FULL:
    case PABB2_CONNECTION_OPCODE_INFO:
    case PABB2_CONNECTION_OPCODE_INFO_U8:
    case PABB2_CONNECTION_OPCODE_INFO_U16:
    case PABB2_CONNECTION_OPCODE_INFO_H32:
    case PABB2_CONNECTION_OPCODE_INFO_U32:
    case PABB2_CONNECTION_OPCODE_INFO_I32:
    case PABB2_CONNECTION_OPCODE_INFO_BINARY:
    case PABB2_CONNECTION_OPCODE_INFO_STR:
    case PABB2_CONNECTION_OPCODE_INFO_LABEL_H32:
    case PABB2_CONNECTION_OPCODE_INFO_LABEL_U32:
    case PABB2_CONNECTION_OPCODE_INFO_LABEL_I32:
//        cout << "Received ack" << endl;
        if (!m_log_everything){
            m_logger.log("[RSC]: Receive: (0x" + tostr_hex(packet->opcode) + ") " + tostr(packet), COLOR_PURPLE);
        }
        return;
    default:
        m_logger.log(
            "[RSC]: UNKNOWN OPCODE: Device sent an unknown opcode: 0x" + tostr_hex(packet->opcode),
            COLOR_RED
        );
        return;
    }
}
void ReliableStreamConnection::process_UNKNOWN_OPCODE(const PacketHeader* packet){
    std::lock_guard<Mutex> lg(m_lock);
    if (packet->packet_bytes < sizeof(PacketHeader_u8) + sizeof(uint32_t)){
        m_error = "Unknown opcode packet is too small: " + std::to_string(packet->packet_bytes);
        m_logger.log("[RSC]: " + m_error, COLOR_RED);
        return;
    }

    const PacketHeader_u8* message = (const PacketHeader_u8*)packet;
    m_logger.log(
        "[RSC]: PABB2_CONNECTION_OPCODE_INVALID_OPCODE: Device reported an invalid opcode: " +
        std::to_string(message->data),
        COLOR_RED
    );
}
void ReliableStreamConnection::process_RET_RESET(const PacketHeader* packet){
    {
        std::lock_guard<Mutex> lg(m_lock);
        m_reliable_sender.remove(packet->seqnum);
    }
    m_cv.notify_all();
}
void ReliableStreamConnection::process_RET_VERSION(const PacketHeader* packet){
//    m_logger.log(tostr(packet), COLOR_DARKGREEN);
    do{
        std::lock_guard<Mutex> lg(m_lock);
        if (packet->packet_bytes < sizeof(PacketHeader_u32) + sizeof(uint32_t)){
            m_error = "Version response is too small: " + std::to_string(packet->packet_bytes);
            m_logger.log("[RSC]: " + m_error, COLOR_RED);
            break;
        }
        m_reliable_sender.remove(packet->seqnum);

        const PacketHeader_u32* message = (const PacketHeader_u32*)packet;
        uint32_t protocol = message->data;
        m_remote_protocol = protocol;
        std::string str = "Remote Protocol: " + std::to_string(protocol);
        uint32_t major_version = protocol / 100;
        uint32_t minor_version = protocol % 100;
        m_remote_protocol_compatible =
            major_version == PABB2_CONNECTION_PROTOCOL_VERSION / 100 &&
            minor_version >= PABB2_CONNECTION_PROTOCOL_VERSION % 100;
        if (!m_remote_protocol_compatible){
            m_error = str + " (incompatible)";
            m_logger.log("[RSC]: " + m_error, COLOR_RED);
            break;
        }

        m_logger.log("[RSC]: " + str + " (compatible)", COLOR_BLUE);
        m_reliable_sender.remove(packet->seqnum);

    }while (false);
    m_cv.notify_all();
}
void ReliableStreamConnection::process_RET_PACKET_SIZE(const PacketHeader* packet){
    if (packet->packet_bytes < sizeof(PacketHeader_u16) + sizeof(uint32_t)){
        m_logger.log(
            "[RSC]: Packet size response is too small: " + std::to_string(packet->packet_bytes),
            COLOR_RED
        );
        return;
    }
    const PacketHeader_u16* message = (const PacketHeader_u16*)packet;
    m_logger.log(
        "[RSC]: Setting Packet Size to: " + std::to_string(message->data),
        COLOR_BLUE
    );
    {
        std::lock_guard<Mutex> lg(m_lock);
        m_reliable_sender.remove(packet->seqnum);
        m_reliable_sender.set_max_packet_size((uint8_t)message->data);
    }
    m_cv.notify_all();
}
void ReliableStreamConnection::process_RET_BUFFER_SLOTS(const PacketHeader* packet){
    if (packet->packet_bytes < sizeof(PacketHeader_u8) + sizeof(uint32_t)){
        m_logger.log(
            "[RSC]: Buffer slot response is too small: " + std::to_string(packet->packet_bytes),
            COLOR_RED
        );
        return;
    }
    const PacketHeader_u8* message = (const PacketHeader_u8*)packet;
    m_logger.log(
        "[RSC]: Setting Buffer Slots to: " + std::to_string(message->data),
        COLOR_BLUE
    );
    {
        std::lock_guard<Mutex> lg(m_lock);
        m_reliable_sender.remove(packet->seqnum);
        m_remote_slot_capacity = std::min<uint8_t>(message->data, PABB2_PacketSender_SLOTS);
    }
    m_cv.notify_all();
}
void ReliableStreamConnection::process_RET_BUFFER_BYTES(const PacketHeader* packet){
    if (packet->packet_bytes < sizeof(PacketHeader_u16) + sizeof(uint32_t)){
        m_logger.log(
            "[RSC]: Buffer slot response is too small: " + std::to_string(packet->packet_bytes),
            COLOR_RED
        );
        return;
    }
    const PacketHeader_u16* message = (const PacketHeader_u16*)packet;
    m_logger.log(
        "[RSC]: Setting Buffer Slots to: " + std::to_string(message->data),
        COLOR_BLUE
    );
    {
        std::lock_guard<Mutex> lg(m_lock);
        m_reliable_sender.remove(packet->seqnum);
        m_remote_buffer_capacity = std::min<uint16_t>(message->data, PABB2_PacketSender_BUFFER_SIZE);
    }
    m_cv.notify_all();
}


void ReliableStreamConnection::process_ASK_STREAM_DATA(const PacketHeader* packet){
//    cout << "process_ASK_STREAM_DATA()" << endl;
    if (packet->packet_bytes < sizeof(PacketHeaderData) + sizeof(uint32_t)){
        m_logger.log(
            "[RSC]: Received stream packet that is too small: " + std::to_string(packet->packet_bytes),
            COLOR_RED
        );
        return;
    }
    if (!m_stream_coalescer.push_stream((const PacketHeaderData*)packet)){
//        cout << "push_stream() failed" << endl;
        return;
    }
//    cout << "Calling: send_ack_u16()" << endl;
    {
        std::lock_guard<Mutex> lg(m_lock);
        send_ack_u16(
            packet->seqnum,
            PABB2_CONNECTION_OPCODE_RET_STREAM_DATA,
            m_stream_coalescer.free_bytes()
        );
    }

    char buffer[4096];
//    m_stream_coalescer.print(false);
    size_t bytes = m_stream_coalescer.read(buffer, sizeof(buffer));
//    cout << "bytes = " << bytes << endl;
    if (bytes != 0){
        on_reliable_recv(buffer, bytes);
    }
}
void ReliableStreamConnection::process_RET_STREAM_DATA(const PacketHeader* packet){
    {
        std::lock_guard<Mutex> lg(m_lock);
        m_reliable_sender.remove(packet->seqnum);
    }
    m_cv.notify_all();
}




}
}
