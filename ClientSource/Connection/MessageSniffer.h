/*  Message Sniffer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_MessageSniffer_H
#define PokemonAutomation_MessageSniffer_H

#include <string>
#include "Common/Compiler.h"

namespace PokemonAutomation{

struct BotBaseMessage;


class MessageSniffer{
public:
    virtual void on_send(const BotBaseMessage& message, bool is_retransmit){}
    virtual void on_recv(const BotBaseMessage& message){}
};



}
#endif
