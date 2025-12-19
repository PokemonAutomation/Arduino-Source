/*  PABotBase Connection
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 *      This class wraps a raw SerialConnection object and applies the
 *  communication protocol on top of it. It automatically throws out bad
 *  messsages and passes only the relevant message body onto the child
 *  listener class.
 * 
 *  This class does not handle retransmissions.
 * 
 */

#ifndef PokemonAutomation_PABotBaseConnection_H
#define PokemonAutomation_PABotBaseConnection_H

#include <memory>
#include <deque>
#include <map>
#include "Common/Cpp/SerialConnection/StreamInterface.h"
#include "Common/SerialPABotBase/SerialPABotBase_Protocol.h"
#include "BotBase.h"

namespace PokemonAutomation{


class BotBaseMessageType;



//  None the functions in this class are thread-safe. It is up to
//  the child class to wrap and make them thread-safe.
class PABotBaseConnection : public StreamListener{
public:
    PABotBaseConnection(Logger& logger, std::unique_ptr<StreamConnection> connection);
    virtual ~PABotBaseConnection();

    void add_message_printer(const BotBaseMessageType& type);

public:
    void send_zeros(uint8_t bytes = PABB_PROTOCOL_MAX_PACKET_SIZE);
    void send_message(const BotBaseMessage& message, bool is_retransmit);

protected:
    //  Not thread-safe with sends.
    void safely_stop();

private:
    virtual void on_recv(const void* data, size_t bytes) override;
    virtual void on_recv_message(BotBaseMessage message) = 0;

    enum class ErrorBatchType{
        NO_ERROR_,
        ZERO_BYTES,
        FF_BYTES,
        ASCII_BYTES,
        OTHER,
    };
    void push_error_byte(ErrorBatchType type, char byte);

private:
    std::unique_ptr<StreamConnection> m_connection;
    std::deque<char> m_recv_buffer;

    ErrorBatchType m_current_error_type;
    std::string m_current_error_batch;

protected:
    Logger& m_logger;

    std::map<uint8_t, const BotBaseMessageType*> m_printers;
};



}

#endif
