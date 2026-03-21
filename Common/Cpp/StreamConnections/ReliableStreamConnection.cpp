/*  Reliable Stream Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/CRC32/pabb_CRC32.h"
//#include "Common/Cpp/PrettyPrint.h"
//#include "Common/Cpp/Exceptions.h"
#include "Common/PABotBase2/ConnectionLayer/PABotBase2_ConnectionDebug.h"
#include "Common/Cpp/StreamConnections/PABotBase2_MessageDumper.h"
#include "ReliableStreamConnection.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



ReliableStreamConnection::ReliableStreamConnection(
    CancellableScope* parent,
    Logger& logger, bool log_everything,
    ThreadPool& thread_pool,
    PokemonAutomation::StreamConnection& unreliable_connection,
    WallDuration retransmit_timeout,
    Mutex* print_lock
)
    : m_logger(logger)
    , m_unreliable_connection(unreliable_connection)
    , m_retransmit_timeout(retransmit_timeout)
    , m_print_lock(print_lock)
    , m_reliable_sender(*this, 20)
    , m_parser(*this)
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
void ReliableStreamConnection::wait_for_pending(){
    std::unique_lock<Mutex> lg(m_lock);
    m_cv.wait(lg, [this]{
        return this->cancelled() || m_reliable_sender.slots_used() == 0;
    });
    throw_if_cancelled();
}



//
//  Send Path
//

void ReliableStreamConnection::reset(){
    {
        std::lock_guard<Mutex> lg(m_lock);
        m_reliable_sender.reset();
        m_parser.reset();
        m_stream_coalescer.reset();
        throw_if_cancelled();
        m_reliable_sender.send_packet(PABB2_CONNECTION_OPCODE_ASK_RESET, 0, nullptr);
    }
    m_cv.notify_all();
    wait_for_pending();
}
size_t ReliableStreamConnection::send(const void* data, size_t bytes){
    std::lock_guard<Mutex> lg(m_lock);
    throw_if_cancelled();
    if (m_reliable_sender.slots_used() >= m_remote_slot_capacity){
        return 0;
    }
    return m_reliable_sender.send_stream(data, bytes);
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
    struct{
        PacketHeader header;
        uint8_t crc[sizeof(uint32_t)];
    } packet;
    packet.header.magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
    packet.header.seqnum = seqnum;
    packet.header.packet_bytes = sizeof(packet);
    packet.header.opcode = opcode;
    pabb_crc32_write_to_message(&packet, sizeof(packet));

    std::lock_guard<Mutex> lg(m_lock);
    m_unreliable_connection.send(&packet, sizeof(packet));
}
void ReliableStreamConnection::send_ack_u16(uint8_t seqnum, uint8_t opcode, uint16_t data){
    struct{
        PacketHeader_Ack_u16 header;
        uint8_t crc[sizeof(uint32_t)];
    } packet;
    packet.header.magic_number = PABB2_CONNECTION_MAGIC_NUMBER;
    packet.header.seqnum = seqnum;
    packet.header.packet_bytes = sizeof(packet);
    packet.header.opcode = opcode;
    packet.header.data = data;
    pabb_crc32_write_to_message(&packet, sizeof(packet));

    std::lock_guard<Mutex> lg(m_lock);
    m_unreliable_connection.send(&packet, sizeof(packet));
}

size_t ReliableStreamConnection::send(const void* data, size_t bytes, bool is_retransmit){
    const PacketHeader* header = (const PacketHeader*)data;
    if (is_retransmit){
        m_logger.log(
            "[ReliableStreamConnection]: Re-send: " + tostr(header),
            COLOR_ORANGE
        );
    }else if (m_log_everything || (
        header->opcode != PABB2_CONNECTION_OPCODE_ASK_STREAM_DATA &&
        header->opcode != PABB2_CONNECTION_OPCODE_RET_STREAM_DATA
    )){
        m_logger.log(
            "[ReliableStreamConnection]: Sending: " + tostr(header),
            COLOR_DARKGREEN
        );
//        PABotBase2::PacketHeader_print(header, true);   //  REMOVE
    }
    return m_unreliable_connection.send(data, bytes);
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
//  Receive Path
//

void ReliableStreamConnection::on_recv(const void* data, size_t bytes){
#if 0
    if (m_print_lock){
        std::lock_guard<Mutex> lg(*m_print_lock);
        cout << "ReliableStreamConnection::on_recv(): " << bytes << endl;
    }
#endif
    m_parser.push_bytes(*this, (const uint8_t*)data, bytes);
}



void ReliableStreamConnection::on_packet(const PacketHeader* packet){
    uint8_t status = packet->magic_number;

//    {
//        std::lock_guard<std::mutex> lg(PokemonAutomation::print_lock);
//        cout << "on_packet(): seqnum = [" << (int)packet->seqnum << "], opcode = " << (int)packet->opcode << endl;
//    }

    switch (status){
    case PABB2_PacketParser_RESULT_VALID:
        break;
    case PABB2_PacketParser_RESULT_INVALID:
        m_logger.log(
            "[ReliableStreamConnection]: INVALID MESSAGE: Received invalid message from device.",
            COLOR_RED
        );
        return;
    case PABB2_PacketParser_RESULT_CHECKSUM_FAIL:
        m_logger.log(
            "[ReliableStreamConnection]: CHECKSUM MISMATCH: Received bad data from device.",
            COLOR_RED
        );
        return;
    default:
        m_logger.log(
            "[ReliableStreamConnection]: Internal Error: Packet parser returned invalid code: " + std::to_string(status),
            COLOR_RED
        );
        return;
    }

    if (m_log_everything){
        m_logger.log("Receive: " + tostr(packet), COLOR_DARKGREEN);
    }


    switch (packet->opcode){
    case PABB2_CONNECTION_OPCODE_INVALID_LENGTH:
        m_logger.log(
            "[ReliableStreamConnection]: PABB2_CONNECTION_OPCODE_INVALID_LENGTH: Device reported an invalid message length.",
            COLOR_RED
        );
        return;
    case PABB2_CONNECTION_OPCODE_INVALID_CHECKSUM_FAIL:
        m_logger.log(
            "[ReliableStreamConnection]: PABB2_CONNECTION_OPCODE_INVALID_CHECKSUM_FAIL: Device reported a checksum mismatch.",
            COLOR_RED
        );
        return;
    case PABB2_CONNECTION_OPCODE_INVALID_OPCODE:
        m_logger.log(
            "[ReliableStreamConnection]: PABB2_CONNECTION_OPCODE_INVALID_OPCODE: Device reported an invalid opcode.",
            COLOR_RED
        );
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
    case PABB2_CONNECTION_OPCODE_INFO:
    case PABB2_CONNECTION_OPCODE_INFO_u8:
    case PABB2_CONNECTION_OPCODE_INFO_u16:
    case PABB2_CONNECTION_OPCODE_INFO_u32:
//        cout << "Received ack" << endl;
        if (!m_log_everything){
            m_logger.log("Receive: " + tostr(packet), COLOR_DARKGREEN);
        }
        return;
    default:
        m_logger.log(
            "[ReliableStreamConnection]: UNKNOWN OPCODE: Device send an unknown opcode: " + std::to_string(packet->opcode),
            COLOR_RED
        );
        return;
    }
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
        if (packet->packet_bytes < sizeof(PacketHeader_Ack_u32) + sizeof(uint32_t)){
            m_error = "Version response is too small: " + std::to_string(packet->packet_bytes);
            m_logger.log("[ReliableStreamConnection]: " + m_error, COLOR_RED);
            break;
        }
        m_reliable_sender.remove(packet->seqnum);

        const PacketHeader_Ack_u32* message = (const PacketHeader_Ack_u32*)packet;
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
            m_logger.log("[ReliableStreamConnection]: " + m_error, COLOR_RED);
            break;
        }

        m_logger.log(
            "[ReliableStreamConnection]: " + str + " (compatible)",
            COLOR_BLUE
        );
        m_reliable_sender.remove(packet->seqnum);

    }while (false);
    m_cv.notify_all();
}
void ReliableStreamConnection::process_RET_PACKET_SIZE(const PacketHeader* packet){
    if (packet->packet_bytes < sizeof(PacketHeader_Ack_u16) + sizeof(uint32_t)){
        m_logger.log(
            "[ReliableStreamConnection]: Packet size response is too small: " + std::to_string(packet->packet_bytes),
            COLOR_RED
        );
        return;
    }
    const PacketHeader_Ack_u16* message = (const PacketHeader_Ack_u16*)packet;
    m_logger.log(
        "[ReliableStreamConnection]: Setting Packet Size to: " + std::to_string(message->data),
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
    if (packet->packet_bytes < sizeof(PacketHeader_Ack_u8) + sizeof(uint32_t)){
        m_logger.log(
            "[ReliableStreamConnection]: Buffer slot response is too small: " + std::to_string(packet->packet_bytes),
            COLOR_RED
        );
        return;
    }
    const PacketHeader_Ack_u8* message = (const PacketHeader_Ack_u8*)packet;
    m_logger.log(
        "[ReliableStreamConnection]: Setting Buffer Slots to: " + std::to_string(message->data),
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
    if (packet->packet_bytes < sizeof(PacketHeader_Ack_u16) + sizeof(uint32_t)){
        m_logger.log(
            "[ReliableStreamConnection]: Buffer slot response is too small: " + std::to_string(packet->packet_bytes),
            COLOR_RED
        );
        return;
    }
    const PacketHeader_Ack_u16* message = (const PacketHeader_Ack_u16*)packet;
    m_logger.log(
        "[ReliableStreamConnection]: Setting Buffer Slots to: " + std::to_string(message->data),
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
    if (packet->packet_bytes < sizeof(PacketHeaderData) + sizeof(uint32_t)){
        m_logger.log(
            "[ReliableStreamConnection]: Received stream packet that is too small: " + std::to_string(packet->packet_bytes),
            COLOR_RED
        );
        return;
    }
    {
        std::lock_guard<Mutex> lg(m_lock);
        if (!m_stream_coalescer.push_stream((const PacketHeaderData*)packet)){
            return;
        }
    }
    send_ack_u16(
        packet->seqnum,
        PABB2_CONNECTION_OPCODE_RET_STREAM_DATA,
        m_stream_coalescer.bytes_available()
    );
    m_cv.notify_all();
}
void ReliableStreamConnection::process_RET_STREAM_DATA(const PacketHeader* packet){
    {
        std::lock_guard<Mutex> lg(m_lock);
//        pabb2_PacketSender_print(&m_reliable_sender, true);
        m_reliable_sender.remove(packet->seqnum);
//        pabb2_PacketSender_print(&m_reliable_sender, true);
    }
    m_cv.notify_all();
}





}
