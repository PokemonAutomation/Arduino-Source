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
#include "Common/Cpp/Exception.h"
#include "Common/Microcontroller/MessageProtocol.h"

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

    template <typename Params>
    BotBaseMessage(uint8_t p_type, const Params& params)
        : type(p_type)
        , body((char*)&params, sizeof(params))
    {}

    template <uint8_t MessageType, typename MessageBody>
    void convert(MessageBody& params) const{
        if (type != MessageType){
            PA_THROW_StringException("Received incorrect response type: " + std::to_string(type));
        }
        if (body.size() != sizeof(MessageBody)){
            PA_THROW_StringException("Received incorrect response size: " + std::to_string(body.size()));
        }
        memcpy(&params, body.c_str(), body.size());
    }

};


class BotBaseRequest{
public:
    BotBaseRequest(bool is_command)
        : m_is_command(is_command)
    {}
    virtual ~BotBaseRequest() = default;
    virtual BotBaseMessage message() const = 0;

    bool is_command() const{ return m_is_command; }

private:
    bool m_is_command;
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
    virtual bool try_issue_request(
        const std::atomic<bool>* cancelled, const BotBaseRequest& request
    ) = 0;
    virtual void issue_request(
        const std::atomic<bool>* cancelled, const BotBaseRequest& request
    ) = 0;
    virtual BotBaseMessage issue_request_and_wait(
        const std::atomic<bool>* cancelled, const BotBaseRequest& request
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





}


#endif

