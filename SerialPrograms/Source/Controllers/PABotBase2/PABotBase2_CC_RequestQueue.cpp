/*  PABotBase2 CC Request Queue
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/CRC32.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Exceptions.h"
#include "PABotBase2_CC_RequestQueue.h"

namespace PokemonAutomation{
namespace PABotBase2{



std::string dump_packet(const pabb2_PacketHeader* packet, bool ascii){
    size_t packet_bytes = packet->packet_bytes;

    std::string ret;
    ret += "{\n";
    ret += "    magic:  0x" + tostr_hex(packet->magic_number) + "\n";
    ret += "    opcode: 0x" + tostr_hex(packet->opcode) + "\n";
    ret += "    bytes:  " + std::to_string(packet_bytes) + "\n";

    if (packet_bytes < sizeof(pabb2_PacketHeader) + sizeof(uint32_t)){
        ret += "    ** invalid length **";
        ret += "\n}";
        return ret;
    }

    uint32_t expected_crc32;
    memcpy(
        &expected_crc32,
        (const uint8_t*)packet + packet_bytes - sizeof(uint32_t),
        sizeof(uint32_t)
    );
    uint32_t actual_crc32 = pabb_crc32(0xffffffff, packet, packet_bytes - sizeof(uint32_t));
    if (actual_crc32 != expected_crc32){
        ret += "    ** CRC mismatch **\n";
    }

    switch (packet->opcode){
    case PABB2_CONNECTION_PACKET_OPCODE_SEND_DATA:{
        constexpr size_t OVERHEAD = sizeof(pabb2_PacketHeaderData) + sizeof(uint32_t);
        const pabb2_PacketHeaderData* stream_packet = (const pabb2_PacketHeaderData*)packet;
        if (packet_bytes < OVERHEAD){
            ret += "    ** invalid length **";
        }else{
            ret += "    offset: " + std::to_string(stream_packet->stream_offset) + "\n";
            ret += "    data:   " + std::string((const char*)(stream_packet + 1), packet_bytes - OVERHEAD);
        }
        break;
    }
    default:
        ret += "   (unknown opcode)";
    }

    ret += "\n}";
    return ret;
}





RequestQueue::RequestQueue(
    Logger& logger,
    StreamSender& sender,
    uint16_t max_packet_size,
    uint32_t receiver_buffer_size
)
    : m_logger(logger)
    , m_sender(sender)
    , m_max_packet_size(max_packet_size)
    , m_receiver_buffer_size(receiver_buffer_size)
    , m_max_data_per_packet(max_packet_size - sizeof(pabb2_PacketHeaderData) - sizeof(uint32_t))
{
    if (m_max_packet_size < m_max_data_per_packet){
        throw InternalProgramError(
            &logger,
            PA_CURRENT_FUNCTION,
            "\"max_packet_size\" is too small."
        );
    }
}

void RequestQueue::reset(){
    size_t crced_data = sizeof(pabb2_PacketHeader);
    size_t packet_size = crced_data + sizeof(uint32_t);

    std::vector<uint8_t> packet(packet_size);

    pabb2_PacketHeader header{
        PABB2_CONNECTION_PACKET_MAGIC_NUMBER,
        PABB2_CONNECTION_PACKET_OPCODE_SEND_DATA,
        (uint16_t)packet_size
    };
    memcpy(packet.data(), &header, sizeof(header));

    uint32_t crc32 = pabb_crc32(0xffffffff, packet.data(), crced_data);
    memcpy(packet.data() + crced_data, &crc32, sizeof(uint32_t));

    std::map<uint32_t, Entry> packets;

    Entry& entry = packets[0];
    entry.stream_bytes = 0;
    entry.packet = std::move(packet);

    //  Now we commit the reset.
    m_unacked_packets = std::move(packets);

    m_sender.send(entry.packet.data(), entry.packet.size());
}

std::string RequestQueue::dump_queue(bool ascii) const{
    std::string str;
    str += "Head: " + std::to_string(m_stream_offset_head) + "\n";
    str += "Tail: " + std::to_string(m_stream_offset_tail) + "\n";
    for (const auto& item : m_unacked_packets){
        str += dump_packet((const pabb2_PacketHeader*)item.second.packet.data() , ascii);
        str += "\n";
    }
    return str;
}


bool RequestQueue::send_data(const uint8_t* data, size_t bytes){
    if (bytes > m_receiver_buffer_size){
        throw InternalProgramError(
            &m_logger,
            PA_CURRENT_FUNCTION,
            "Data is larger than the receiver's buffer size."
        );
    }
    if (m_stream_offset_tail - m_stream_offset_head + bytes > m_receiver_buffer_size){
        return false;
    }

    if (bytes <= m_max_data_per_packet){
        send_data_small(data, bytes);
    }else{
        send_data_large(data, bytes);
    }

    return true;
}

void RequestQueue::report_acked(uint32_t stream_offset){
    auto iter = m_unacked_packets.find(stream_offset);
    if (iter == m_unacked_packets.end()){
        m_logger.log("Ignoring ack for unknown offset: " + tostr_u_commas(stream_offset), COLOR_ORANGE);
        return;
    }

    iter = m_unacked_packets.erase(iter);

    //  Not in the front.
    if (stream_offset != m_stream_offset_head){
        return;
    }

    if (m_unacked_packets.empty()){
        m_stream_offset_head = m_stream_offset_tail;
        return;
    }

    //  Wrap around
    if (iter == m_unacked_packets.end()){
        iter = m_unacked_packets.begin();
    }

    m_stream_offset_head = iter->first;
}


std::vector<uint8_t> RequestQueue::make_data_packet(
    uint32_t stream_offset,
    const uint8_t* data, size_t bytes
) const{
    size_t crced_data = bytes + sizeof(pabb2_PacketHeaderData);
    size_t packet_size = crced_data + sizeof(uint32_t);

    std::vector<uint8_t> packet(packet_size);

    pabb2_PacketHeaderData header{
        PABB2_CONNECTION_PACKET_MAGIC_NUMBER,
        PABB2_CONNECTION_PACKET_OPCODE_SEND_DATA,
        (uint16_t)packet_size,
        stream_offset,
    };
    memcpy(packet.data(), &header, sizeof(header));

    memcpy(packet.data() + sizeof(header), data, bytes);

    uint32_t crc32 = pabb_crc32(0xffffffff, packet.data(), crced_data);
    memcpy(packet.data() + crced_data, &crc32, sizeof(uint32_t));

    return packet;
}

void RequestQueue::send_data_small(const uint8_t* data, size_t bytes){
    std::vector<uint8_t> packet = make_data_packet(m_stream_offset_tail, data, bytes);

    //  Commit the packet.
    Entry& entry = m_unacked_packets[m_stream_offset_tail];
    entry.packet = std::move(packet);
    entry.stream_bytes = bytes;
    m_stream_offset_tail += (uint32_t)bytes;

    m_sender.send(entry.packet.data(), entry.packet.size());
}

void RequestQueue::send_data_large(const uint8_t* data, size_t bytes){
    std::vector<std::map<uint32_t, Entry>::iterator> nodes;

    uint32_t tail = m_stream_offset_tail;

    try{
        size_t max_data_per_packet = m_max_data_per_packet;
        while (bytes > 0){
            size_t block = std::min(bytes, max_data_per_packet);

            auto& iter = nodes.emplace_back(m_unacked_packets.end());

            auto ret = m_unacked_packets.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(tail),
                std::forward_as_tuple(block, make_data_packet(tail, data, block))
            );
            if (!ret.second){
                throw InternalProgramError(
                    &m_logger,
                    PA_CURRENT_FUNCTION,
                    "Duplicate Stream Offset: " + tostr_hex(tail)
                );
            }

            iter = ret.first;

            tail += (uint32_t)block;
            data += block;
            bytes -= block;
        }
    }catch (...){
        for (auto& item : nodes){
            if (item != m_unacked_packets.end()){
                m_unacked_packets.erase(item);
            }
        }
        throw;
    }

    m_stream_offset_tail = tail;
    for (auto& item : nodes){
        m_sender.send(item->second.packet.data(), item->second.packet.size());
    }
}


















}
}
