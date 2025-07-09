/*  Pokemon Automation Bot Base - Client Example
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#include "Common/CRC32.h"
#include "Common/SerialPABotBase/SerialPABotBase_Protocol.h"
#include "ClientSource/Libraries/Logging.h"
#include "ClientSource/Libraries/MessageConverter.h"
#include "BotBaseMessage.h"
#include "PABotBaseConnection.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{

MessageSniffer null_sniffer;

PABotBaseConnection::PABotBaseConnection(Logger& logger, std::unique_ptr<StreamConnection> connection)
    : m_connection(std::move(connection))
    , m_logger(logger)
    , m_sniffer(&null_sniffer)
{
    m_connection->add_listener(*this);
}
PABotBaseConnection::~PABotBaseConnection(){
    if (m_connection){
        safely_stop();
    }
}
void PABotBaseConnection::safely_stop(){
    if (m_connection){
        m_connection->stop();
        m_connection.reset();
    }
}


void PABotBaseConnection::set_sniffer(MessageSniffer* sniffer){
    if (sniffer == nullptr){
        sniffer = &null_sniffer;
    }
    m_sniffer = sniffer;
}


void PABotBaseConnection::send_zeros(uint8_t bytes){
    if (!m_connection){
        return;
    }

    char ch = 0;
    for (uint8_t c = 0; c < bytes; c++){
        m_connection->send(&ch, 1);
//        Sleep(10);
    }
}
void PABotBaseConnection::send_message(const BotBaseMessage& message, bool is_retransmit){
    if (!m_connection){
        return;
    }

//    log("Sending: " + message_to_string(type, msg));
    m_sniffer->on_send(message, is_retransmit);

    size_t total_bytes = PABB_PROTOCOL_OVERHEAD + message.body.size();
    if (total_bytes > PABB_PROTOCOL_MAX_PACKET_SIZE){
        throw InternalProgramError(&m_logger, PA_CURRENT_FUNCTION, "Message is too long.");
    }

    std::string buffer;
    buffer += ~(uint8_t)total_bytes;
    buffer += message.type;
    buffer += message.body;
    buffer += std::string(sizeof(uint32_t), 0);
    pabb_crc32_write_to_message(&buffer[0], buffer.size());

    m_connection->send(&buffer[0], buffer.size());
}


void PABotBaseConnection::on_recv(const void* data, size_t bytes){
    //  Push into receive buffer.
    for (size_t c = 0; c < bytes; c++){
        m_recv_buffer.emplace_back(((const char*)data)[c]);
    }

    while (!m_recv_buffer.empty()){
        uint8_t length = ~m_recv_buffer[0];

        if (m_recv_buffer[0] == 0){
            m_sniffer->log("Skipping zero byte.");
            m_recv_buffer.pop_front();
            continue;
        }

        //  Message is too short.
        if (length < PABB_PROTOCOL_OVERHEAD){
            m_sniffer->log("Message is too short: bytes = " + std::to_string(length));
            m_recv_buffer.pop_front();
            continue;
        }

        //  Message is too long.
        if (length > PABB_PROTOCOL_MAX_PACKET_SIZE){
            char ascii = ~length;
            std::string text = ascii < 32
                ? ", ascii = " + std::to_string(ascii)
                : std::string(", char = ") + ascii;
            m_sniffer->log("Message is too long: bytes = " + std::to_string(length) + text);
            m_recv_buffer.pop_front();
            continue;
        }

        //  Message is incomplete.
        if (length > m_recv_buffer.size()){
            return;
        }

        std::string message(m_recv_buffer.begin(), m_recv_buffer.begin() + length);

        //  Verify checksum
        {
            //  Calculate checksum.
            uint32_t checksumA = pabb_crc32(0xffffffff, &message[0], length - sizeof(uint32_t));

            //  Read the checksum from the message.
            uint32_t checksumE = ((uint32_t*)(&message[0] + length))[-1];

            //  Compare
//            std::cout << checksumA << " / " << checksumE << std::endl;
            if (checksumA != checksumE){
                m_sniffer->log("Invalid Checksum: bytes = " + std::to_string(length));
//                std::cout << checksumA << " / " << checksumE << std::endl;
//                log(message_to_string(message[1], &message[2], length - PABB_PROTOCOL_OVERHEAD));
                m_recv_buffer.pop_front();
                continue;
            }
        }
        m_recv_buffer.erase(m_recv_buffer.begin(), m_recv_buffer.begin() + length);

        BotBaseMessage msg(message[1], std::string(&message[2], length - PABB_PROTOCOL_OVERHEAD));
        m_sniffer->on_recv(msg);
        on_recv_message(std::move(msg));
    }
}


}
