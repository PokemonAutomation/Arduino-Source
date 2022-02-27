/*  Pokemon Automation Bot-Base Interface
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AbstractBotBase_H
#define PokemonAutomation_AbstractBotBase_H

#include <atomic>

namespace PokemonAutomation{


//  This is the universal "operation cancelled" exception that is used to break
//  out of routines in a clean manner when a users requests a cancellation.
struct CancelledException{
    CancelledException(){}
};


struct BotBaseMessage;
class BotBaseRequest;



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
    virtual void wait_for_all_requests(const std::atomic<bool>* cancelled = nullptr) = 0;
    virtual void stop_all_commands() = 0;

public:
    virtual bool try_issue_request(
        const BotBaseRequest& request,
        const std::atomic<bool>* cancelled = nullptr
    ) = 0;
    virtual void issue_request(
        const BotBaseRequest& request,
        const std::atomic<bool>* cancelled = nullptr
    ) = 0;
    virtual BotBaseMessage issue_request_and_wait(
        const BotBaseRequest& request,
        const std::atomic<bool>* cancelled = nullptr
    ) = 0;

};



//  A wrapper for BotBase that allows for asynchronous cancelling.
class BotBaseContext{
public:
    BotBaseContext(BotBase& botbase)
        : m_botbase(botbase)
        , m_cancelled(false)
    {}

    void wait_for_all_requests() const{
        m_botbase.wait_for_all_requests(&m_cancelled);
    }

    //  Don't use this unless you really need to.
    BotBase& botbase() const{ return m_botbase; }
//    BotBase* operator->() const{
//        check_cancelled();
//        return &m_botbase;
//    }

    void check_cancelled() const{
        if (m_cancelled.load(std::memory_order_acquire)){
            throw CancelledException();
        }
    }
    void cancel(){
        m_cancelled.store(true, std::memory_order_release);
        m_botbase.stop_all_commands();
    }

    const std::atomic<bool>& cancelled_bool() const{
        return m_cancelled;
    }


public:
    bool try_issue_request(const BotBaseRequest& request) const{
        return m_botbase.try_issue_request(request, &m_cancelled);
    }
    void issue_request(const BotBaseRequest& request) const{
        m_botbase.issue_request(request, &m_cancelled);
    }
    BotBaseMessage issue_request_and_wait(const BotBaseRequest& request) const;


private:
    BotBase& m_botbase;
    std::atomic<bool> m_cancelled;
};





}


#endif

