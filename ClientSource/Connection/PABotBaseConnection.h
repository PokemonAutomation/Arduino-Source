/*  PABotBase Connection
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
#include <string>
#include <deque>
#include "Common/Compiler.h"
#include "Common/MessageProtocol.h"
#include "ClientSource/Libraries/Compiler.h"
#include "BotBase.h"
#include "StreamInterface.h"

namespace PokemonAutomation{


class MessageSniffer{
public:
    virtual void log(std::string msg){}
    virtual void on_send(const BotBaseMessage& message, bool is_retransmit){}
    virtual void on_recv(const BotBaseMessage& message){}
};


class PABotBaseConnection : public StreamListener{
public:
    PABotBaseConnection(std::unique_ptr<StreamConnection> connection);
    virtual ~PABotBaseConnection();

    void set_sniffer(MessageSniffer* sniffer);

public:
    void send_zeros(uint8_t bytes = PABB_MAX_PACKET_SIZE);
    void send_message(const BotBaseMessage& message, bool is_retransmit);

protected:
    //  Not thread-safe with sends.
    void safely_stop();

private:
    virtual void on_recv(const void* data, size_t bytes) override;
    virtual void on_recv_message(BotBaseMessage message) = 0;

private:
    std::unique_ptr<StreamConnection> m_connection;
    std::deque<char> m_recv_buffer;

protected:
    MessageSniffer* m_sniffer;
};



}

#endif
