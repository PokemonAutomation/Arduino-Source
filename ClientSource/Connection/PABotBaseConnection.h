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
//#include "Common/SerialPABotBase/SerialPABotBase_Protocol.h"
#include "BotBase.h"
#include "MessageSniffer.h"
#include "StreamInterface.h"

namespace PokemonAutomation{


//  None the functions in this class are thread-safe. It is up to
//  the child class to wrap and make them thread-safe.
class PABotBaseConnection : public StreamListener{
public:
    static const size_t MAX_MESSAGE_SIZE = 64;

public:
    PABotBaseConnection(Logger& logger, std::unique_ptr<StreamConnection> connection);
    virtual ~PABotBaseConnection();

    void set_sniffer(MessageSniffer* sniffer);

public:
    void send_zeros(uint8_t bytes = MAX_MESSAGE_SIZE);
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
    Logger& m_logger;
    MessageSniffer* m_sniffer;
};



}

#endif
