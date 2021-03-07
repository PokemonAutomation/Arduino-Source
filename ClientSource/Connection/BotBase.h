/*  Pokemon Automation Bot-Base Interface
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AbstractBotBase_H
#define PokemonAutomation_AbstractBotBase_H

#include <stdint.h>
#include <string>
#include "Common/MessageProtocol.h"

namespace PokemonAutomation{

//  This is the universal "operation cancelled" exception that is used to break
//  out of routines in a clean manner when a users requests a cancellation.
struct CancelledException{
    CancelledException(){}
};


struct BotBaseMessage{
    uint8_t type;
    std::string body;

    BotBaseMessage() = default;
    BotBaseMessage(uint8_t p_type, std::string p_body)
        : type(p_type)
        , body(std::move(p_body))
    {}
};


class BotBase{
public:
    enum class State{
        RUNNING,
        NO_COMMANDS,
        STOPPING,
        STOPPED,
    };

public:
    virtual ~BotBase() = default;
    virtual State state() const = 0;
    virtual void wait_for_all_requests() = 0;

public:
    //  Request Dispatch

    //  Return if request cannot be dispatched immediately.
    template <uint8_t SendType, typename SendParams>
    bool try_issue_request(SendParams& send_params);

    //  Block the thread until the request is sent.
    template <uint8_t SendType, typename SendParams>
    void issue_request(SendParams& send_params);

    //  Block the thread until the request is send and the response is received.
    template <
        uint8_t SendType, uint8_t RecvType,
        typename SendParams, typename RecvParams
    >
    void issue_request_and_wait(SendParams& send_params, RecvParams& recv_params);


protected:
    virtual bool try_issue_request(
        uint8_t send_type, char* send_params, size_t send_bytes
    ) = 0;
    virtual void issue_request(
        uint8_t send_type, char* send_params, size_t send_bytes
    ) = 0;
    virtual void issue_request_and_wait(
        uint8_t send_type, char* send_params, size_t send_bytes,
        uint8_t recv_type, char* recv_params, size_t recv_bytes
    ) = 0;
};




//  Implementations

template <uint8_t SendType, typename SendParams>
bool BotBase::try_issue_request(SendParams& send_params){
    static_assert(sizeof(SendParams) <= PABB_MAX_MESSAGE_SIZE, "Message is too large.");
    return try_issue_request(SendType, (char*)&send_params, sizeof(SendParams));
}
template <uint8_t SendType, typename SendParams>
void BotBase::issue_request(SendParams& send_params){
    static_assert(sizeof(SendParams) <= PABB_MAX_MESSAGE_SIZE, "Message is too large.");
    issue_request(SendType, (char*)&send_params, sizeof(SendParams));
}
template <
    uint8_t SendType, uint8_t RecvType,
    typename SendParams, typename RecvParams
>
void BotBase::issue_request_and_wait(SendParams& send_params, RecvParams& recv_params){
    static_assert(sizeof(SendParams) <= PABB_MAX_MESSAGE_SIZE, "Message is too large.");
    static_assert(sizeof(RecvParams) <= PABB_MAX_MESSAGE_SIZE, "Message is too large.");
    static_assert(PABB_MSG_IS_REQUEST(SendType), "Message must be a request.");
    issue_request_and_wait(
        SendType, (char*)&send_params, sizeof(SendParams),
        RecvType, (char*)&recv_params, sizeof(RecvParams)
    );
}





}


#endif

