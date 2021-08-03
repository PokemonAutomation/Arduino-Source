/*  Pokemon Automation Bot-Base Interface
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AbstractBotBase_H
#define PokemonAutomation_AbstractBotBase_H

#include <stdint.h>
#include <string>
#include <atomic>
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
        STOPPING,
        STOPPED,
    };

public:
    virtual ~BotBase() = default;
    virtual State state() const = 0;
    virtual void wait_for_all_requests() = 0;
    virtual void stop_all_commands() = 0;

public:
    //  Request Dispatch

    //  Return if request cannot be dispatched immediately.
    template <uint8_t SendType, typename SendParams>
    bool try_issue_request(
        const std::atomic<bool>* cancelled,
        SendParams& send_params
    );

    //  Block the thread until the request is sent.
    template <uint8_t SendType, typename SendParams>
    void issue_request(
        const std::atomic<bool>* cancelled,
        SendParams& send_params
    );

    //  Block the thread until the request is sent and the response is received.
    template <
        uint8_t SendType, uint8_t RecvType,
        typename SendParams, typename RecvParams
    >
    void issue_request_and_wait(
        const std::atomic<bool>* cancelled,
        SendParams& send_params,
        RecvParams& recv_params
    );


public:
    virtual bool try_issue_request(
        const std::atomic<bool>* cancelled,
        uint8_t send_type, void* send_params, size_t send_bytes
    ) = 0;
    virtual void issue_request(
        const std::atomic<bool>* cancelled,
        uint8_t send_type, void* send_params, size_t send_bytes
    ) = 0;
    virtual void issue_request_and_wait(
        const std::atomic<bool>* cancelled,
        uint8_t send_type, void* send_params, size_t send_bytes,
        uint8_t recv_type, void* recv_params, size_t recv_bytes
    ) = 0;
};



//  A wrapper for BotBase that allows for asynchronous cancelling.
class BotBaseContext{
public:
    BotBaseContext(BotBase& botbase)
        : m_botbase(&botbase)
        , m_cancelled(false)
    {}

    BotBase& botbase() const{ return *m_botbase; }
//    operator BotBase&() const{
//        return *m_botbase;
//    }
    BotBase* operator->() const{
        check_cancelled();
        return m_botbase;
    }
    void check_cancelled() const{
        if (m_cancelled.load(std::memory_order_acquire)){
            throw CancelledException();
        }
    }
    void cancel(){
        m_cancelled.store(true, std::memory_order_release);
        m_botbase->stop_all_commands();
    }

    const std::atomic<bool>& cancelled_bool() const{
        return m_cancelled;
    }

private:
    BotBase* m_botbase;
    std::atomic<bool> m_cancelled;
};






//  Implementations

template <uint8_t SendType, typename SendParams>
bool BotBase::try_issue_request(
    const std::atomic<bool>* cancelled,
    SendParams& send_params
){
    static_assert(sizeof(SendParams) <= PABB_MAX_MESSAGE_SIZE, "Message is too large.");
    return try_issue_request(cancelled, SendType, &send_params, sizeof(SendParams));
}
template <uint8_t SendType, typename SendParams>
void BotBase::issue_request(
    const std::atomic<bool>* cancelled,
    SendParams& send_params
){
    static_assert(sizeof(SendParams) <= PABB_MAX_MESSAGE_SIZE, "Message is too large.");
    issue_request(cancelled, SendType, &send_params, sizeof(SendParams));
}
template <
    uint8_t SendType, uint8_t RecvType,
    typename SendParams, typename RecvParams
>
void BotBase::issue_request_and_wait(
    const std::atomic<bool>* cancelled,
    SendParams& send_params,
    RecvParams& recv_params
){
    static_assert(sizeof(SendParams) <= PABB_MAX_MESSAGE_SIZE, "Message is too large.");
    static_assert(sizeof(RecvParams) <= PABB_MAX_MESSAGE_SIZE, "Message is too large.");
    static_assert(PABB_MSG_IS_REQUEST(SendType), "Message must be a request.");
    issue_request_and_wait(
        cancelled,
        SendType, &send_params, sizeof(SendParams),
        RecvType, &recv_params, sizeof(RecvParams)
    );
}





}


#endif

