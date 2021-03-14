/*  Pokemon Automation Bot Base
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <algorithm>
#include <iostream>
#include <emmintrin.h>
#include "Common/MessageProtocol.h"
#include "PABotBase.h"

namespace PokemonAutomation{



PABotBase::PABotBase(
    std::unique_ptr<StreamConnection> connection,
    MessageLogger* logger,
    std::chrono::milliseconds retransmit_delay
)
    : PABotBaseConnection(std::move(connection))
    , m_send_seq(1)
    , m_retransmit_delay(retransmit_delay)
    , m_last_ack(std::chrono::system_clock::now())
    , m_state(State::RUNNING)
    , m_retransmit_thread(&PABotBase::retransmit_thread, this)
{
    set_sniffer(logger);
}
PABotBase::~PABotBase(){
    stop();
    while (m_state.load(std::memory_order_acquire) != State::STOPPED){
        _mm_pause();
    }
}
void PABotBase::connect(){
    //  Send seqnum reset.
    pabb_MsgInfoSeqnumReset params;
    pabb_MsgAckRequest response;
    issue_request_and_wait<PABB_MSG_SEQNUM_RESET, PABB_MSG_ACK_REQUEST>(params, response);
}
void PABotBase::stop(){
//    cout << "stop" << endl;

    //  Make sure only one thread can get in here.
    State expected = State::RUNNING;
    if (!m_state.compare_exchange_strong(expected, State::STOPPING)){
        return;
    }

    //  Wake everyone up.
    {
        std::lock_guard<std::mutex> lg(m_sleep_lock);
        m_cv.notify_all();
    }
    m_retransmit_thread.join();

    SpinLockGuard lg(m_state_lock, "PABotBase::stop()");

    //  Send a stop request, but don't wait for a response that we may never
    //  receive.
    pabb_MsgRequestStop params;
    uint64_t seqnum = m_send_seq;
    seqnum_t seqnum_s = (seqnum_t)seqnum;
    memcpy(&params, &seqnum_s, sizeof(seqnum_t));
//    try_issue_request<PABB_MSG_REQUEST_STOP>(params);
//    m_state.store(State::STOPPING, std::memory_order_release);
    BotBaseMessage stop_request(PABB_MSG_REQUEST_STOP, std::string((char*)&params, sizeof(params)));
    send_message(stop_request, false);

    //  Must call this to stop the receiver thread from making any more async
    //  calls into this class which touch its fields.
    safely_stop();

    //  Now the receiver thread is dead. Nobody else is touching this class so
    //  it is safe to destruct.
    m_state.store(State::STOPPED, std::memory_order_release);
}

void PABotBase::wait_for_all_requests(){
    std::unique_lock<std::mutex> lg(m_sleep_lock);
    bool canceled = false;
    m_cv.wait(lg, [&]{
        SpinLockGuard lg(m_state_lock, "PABotBase::wait_for_all_requests()");
        if (m_state.load(std::memory_order_acquire) != State::RUNNING){
            canceled = true;
            return true;
        }
        return m_pending_requests.empty() && m_pending_commands.empty();
    });
    if (canceled){
        throw CancelledException();
    }
}
void PABotBase::remove_request(std::map<uint64_t, PendingRequest>::iterator iter){
    //  Must be called under both sleep and state locks.
    m_pending_requests.erase(iter);
    m_cv.notify_all();
}
void PABotBase::remove_command(std::map<uint64_t, PendingCommand>::iterator iter){
    //  Must be called under both sleep and state locks.
    m_pending_commands.erase(iter);
    m_cv.notify_all();
}
template <typename Map>
uint64_t PABotBase::infer_full_seqnum(const Map& map, seqnum_t seqnum) const{
    //  The protocol uses a 32-bit seqnum that wraps around. For our purposes of
    //  retransmits, we use a full 64-bit seqnum to maintain sorting order
    //  across the wrap-arounds.

    //  Since the oldest unacked messaged will never be more than 63 (MAX_SEQNUM_GAP)
    //  requests old, there is no ambiguity on which request is being referred
    //  to with just the lower 32 bits.
    //  Here we infer the upper 32 bits of the seqnum to obtain the full 64-bit
    //  seqnum that we need to index our map.

    //  This needs to be called inside the lock. Furthermore, the map must not
    //  be empty. If it is empty, we know we don't have it and can drop it
    //  before we even call this function.

    //  Figure out the upper 32 bits of the seqnum.
    uint64_t lo = map.begin()->first;
    uint64_t hi = map.rbegin()->first;
    uint64_t lo_candidate = (lo & 0xffffffff00000000) | seqnum;
    uint64_t hi_candidate = (hi & 0xffffffff00000000) | seqnum;
    return lo_candidate >= lo
        ? lo_candidate
        : hi_candidate;
}

uint64_t PABotBase::oldest_live_seqnum() const{
    //  Must call under state lock.
    uint64_t oldest = m_send_seq;
    if (!m_pending_requests.empty()){
        oldest = std::min(oldest, m_pending_requests.begin()->first);
    }
    if (!m_pending_commands.empty()){
        oldest = std::min(oldest, m_pending_commands.begin()->first);
    }
    return oldest;
}

template <typename Params>
void PABotBase::process_ack_request(BotBaseMessage message){
    if (message.body.size() != sizeof(Params)){
        m_sniffer->log("Ignoring message with invalid size.");
        return;
    }
    const Params* params = (const Params*)message.body.c_str();
    seqnum_t seqnum = params->seqnum;

    std::map<uint64_t, PendingRequest>::iterator iter;
    {
        SpinLockGuard lg(m_state_lock, "PABotBase::process_ack_request() - 0");
        if (m_pending_requests.empty()){
            m_sniffer->log("Unexpected request ack message: seqnum = " + std::to_string(seqnum));
            return;
        }

        uint64_t full_seqnum = infer_full_seqnum(m_pending_requests, seqnum);
        iter = m_pending_requests.find(full_seqnum);
        if (iter == m_pending_requests.end()){
            m_sniffer->log("Unexpected request ack message: seqnum = " + std::to_string(seqnum));
            return;
        }
    }

    m_last_ack.store(std::chrono::system_clock::now(), std::memory_order_release);

    switch (iter->second.state){
    case AckState::NOT_ACKED:
//        std::cout << "acked: " << full_seqnum << std::endl;
        iter->second.state = AckState::ACKED;
        iter->second.ack = std::move(message);
        if (iter->second.silent_remove){
            SpinLockGuard lg(m_state_lock, "PABotBase::process_ack_request() - 1");
            m_pending_requests.erase(iter);
        }
        {
            std::lock_guard<std::mutex> lg(m_sleep_lock);
            m_cv.notify_all();
        }
        return;
    case AckState::ACKED:
        m_sniffer->log("Duplicate request ack message: seqnum = " + std::to_string(seqnum));
        return;
    case AckState::FINISHED:
        m_sniffer->log("Request ack on command finish: seqnum = " + std::to_string(seqnum));
        return;
    }
}
template <typename Params>
void PABotBase::process_ack_command(BotBaseMessage message){
    if (message.body.size() != sizeof(Params)){
        m_sniffer->log("Ignoring message with invalid size.");
        return;
    }
    const Params* params = (const Params*)message.body.c_str();
    seqnum_t seqnum = params->seqnum;

    std::map<uint64_t, PendingCommand>::iterator iter;
    {
        SpinLockGuard lg(m_state_lock, "PABotBase::process_ack_command()");
        if (m_pending_commands.empty()){
            m_sniffer->log("Unexpected command ack message: seqnum = " + std::to_string(seqnum));
            return;
        }

        uint64_t full_seqnum = infer_full_seqnum(m_pending_commands, seqnum);
        iter = m_pending_commands.find(full_seqnum);
        if (iter == m_pending_commands.end()){
            m_sniffer->log("Unexpected command ack message: seqnum = " + std::to_string(seqnum));
            return;
        }
    }

    m_last_ack.store(std::chrono::system_clock::now(), std::memory_order_release);

    switch (iter->second.state){
    case AckState::NOT_ACKED:
//        std::cout << "acked: " << full_seqnum << std::endl;
        iter->second.state = AckState::ACKED;
        iter->second.ack = std::move(message);
        return;
    case AckState::ACKED:
        m_sniffer->log("Duplicate command ack message: seqnum = " + std::to_string(seqnum));
        return;
    case AckState::FINISHED:
        m_sniffer->log("Command ack on finished command: seqnum = " + std::to_string(seqnum));
        return;
    }
}
template <typename Params>
void PABotBase::process_command_finished(BotBaseMessage message){
    if (message.body.size() != sizeof(Params)){
        m_sniffer->log("Ignoring message with invalid size.");
        return;
    }
    const Params* params = (const Params*)message.body.c_str();
    seqnum_t seqnum = params->seqnum;
    seqnum_t command_seqnum = params->seq_of_original_command;

    //  Send the ack first.
    pabb_MsgAckRequest ack;
    ack.seqnum = seqnum;
//    m_send_queue.emplace_back((uint8_t)PABB_MSG_ACK, std::string((char*)&ack, sizeof(ack)));
    send_message(BotBaseMessage(PABB_MSG_ACK_REQUEST, std::string((char*)&ack, sizeof(ack))), false);

    std::map<uint64_t, PendingCommand>::iterator iter;
    {
        SpinLockGuard lg(m_state_lock, "PABotBase::process_command_finished() - 0");
        if (m_pending_commands.empty()){
            m_sniffer->log(
                "Unexpected command finished message: seqnum = " + std::to_string(seqnum) +
                ", command_seqnum = " + std::to_string(command_seqnum)
            );
            return;
        }

        uint64_t full_seqnum = infer_full_seqnum(m_pending_commands, command_seqnum);
        iter = m_pending_commands.find(full_seqnum);
        if (iter == m_pending_commands.end()){
            m_sniffer->log(
                "Unexpected command finished message: seqnum = " + std::to_string(seqnum) +
                ", command_seqnum = " + std::to_string(command_seqnum)
            );
            return;
        }
    }

    switch (iter->second.state){
    case AckState::NOT_ACKED:
    case AckState::ACKED:
        iter->second.state = AckState::FINISHED;
        iter->second.ack = std::move(message);
        if (iter->second.silent_remove){
            SpinLockGuard lg(m_state_lock, "PABotBase::process_command_finished() - 1");
            m_pending_commands.erase(iter);
        }
        {
            std::lock_guard<std::mutex> lg(m_sleep_lock);
            m_cv.notify_all();
        }
        return;
    case AckState::FINISHED:
        m_sniffer->log("Duplicate command finish: seqnum = " + std::to_string(seqnum));
        return;
    }
}
void PABotBase::on_recv_message(BotBaseMessage message){
    switch (message.type){
    case PABB_MSG_ACK_COMMAND:
        process_ack_command<pabb_MsgAckCommand>(std::move(message));
        return;
    case PABB_MSG_ACK_REQUEST:
        process_ack_request<pabb_MsgAckRequest>(std::move(message));
        return;
    case PABB_MSG_ACK_REQUEST_I8:
        process_ack_request<pabb_MsgAckRequestI8>(std::move(message));
        return;
    case PABB_MSG_ACK_REQUEST_I16:
        process_ack_request<pabb_MsgAckRequestI16>(std::move(message));
        return;
    case PABB_MSG_ACK_REQUEST_I32:
        process_ack_request<pabb_MsgAckRequestI32>(std::move(message));
        return;
//    case PABB_MSG_INFO_COMMAND_DROPPED:
//        return;

    case PABB_MSG_REQUEST_COMMAND_FINISHED:{
        process_command_finished<pabb_MsgRequestCommandFinished>(std::move(message));
        return;
    }
    }
}

void PABotBase::retransmit_thread(){
//    cout << "retransmit_thread()" << endl;
    auto last_sent = std::chrono::system_clock::now();
    while (m_state.load(std::memory_order_acquire) == State::RUNNING){
        auto now = std::chrono::system_clock::now();

        if (now - last_sent < m_retransmit_delay){
            std::unique_lock<std::mutex> lg(m_sleep_lock);
            if (m_state.load(std::memory_order_acquire) != State::RUNNING){
                break;
            }
            m_cv.wait_for(lg, m_retransmit_delay);
            continue;
        }

        //  Process retransmits.
        std::map<uint64_t, PendingRequest> pending_requests;
        std::map<uint64_t, PendingCommand> pending_commands;
        {
            SpinLockGuard lg(m_state_lock, "PABotBase::retransmit_thread()");
            pending_requests = m_pending_requests;
            pending_commands = m_pending_commands;
        }
//        std::cout << "retransmit_thread - m_pending_messages.size(): " << m_pending_messages.size() << std::endl;
//        cout << "m_pending_messages.size()" << endl;

        //  Retransmit
        //      Iterate through all pending requests and retransmit them in
        //  chronological order. Skip the ones that are new.
        for (auto iter : pending_requests){
            if (
                iter.second.state == AckState::NOT_ACKED &&
                std::chrono::system_clock::now() - iter.second.first_sent >= m_retransmit_delay
            ){
                send_message(iter.second.request, true);
            }
        }
        for (auto iter : pending_commands){
            if (
                iter.second.state == AckState::NOT_ACKED &&
                std::chrono::system_clock::now() - iter.second.first_sent >= m_retransmit_delay
            ){
                send_message(iter.second.request, true);
            }
        }
        last_sent = std::chrono::system_clock::now();
    }
//    cout << "retransmit_thread() - exit" << endl;
}




bool PABotBase::try_issue_request(
    std::map<uint64_t, PendingRequest>::iterator& iter,
    uint8_t send_type, char* send_params, size_t send_bytes,
    bool silent_remove, size_t queue_limit
){
    if (send_bytes > PABB_MAX_MESSAGE_SIZE){
        throw "Message is too long.";
    }

    SpinLockGuard lg(m_state_lock, "PABotBase::try_issue_request()");

    State state = m_state.load(std::memory_order_acquire);
    if (state != State::RUNNING){
        throw CancelledException();
    }

    //  Don't get too far ahead of the oldest seqnum.
    uint64_t seqnum = m_send_seq;
    if (seqnum - oldest_live_seqnum() > MAX_SEQNUM_GAP){
        return false;
    }

    seqnum_t seqnum_s = (seqnum_t)seqnum;
    memcpy(send_params, &seqnum_s, sizeof(seqnum_t));

    std::pair<std::map<uint64_t, PendingRequest>::iterator, bool> ret = m_pending_requests.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(seqnum),
        std::forward_as_tuple()
    );
    if (!ret.second){
        throw "Duplicate sequence number: " + std::to_string(seqnum);
    }

    m_send_seq = seqnum + 1;

    PendingRequest* handle = &ret.first->second;

    handle->silent_remove = silent_remove;
    handle->request = BotBaseMessage(send_type, std::string(send_params, send_bytes));
    handle->first_sent = std::chrono::system_clock::now();

    send_message(handle->request, false);

    iter = ret.first;

    return true;
}
bool PABotBase::try_issue_command(
    std::map<uint64_t, PendingCommand>::iterator& iter,
    uint8_t send_type, char* send_params, size_t send_bytes,
    bool silent_remove, size_t queue_limit
){
    if (send_bytes > PABB_MAX_MESSAGE_SIZE){
        throw "Message is too long.";
    }

    SpinLockGuard lg(m_state_lock, "PABotBase::try_issue_command()");

    State state = m_state.load(std::memory_order_acquire);
    if (state != State::RUNNING){
        throw CancelledException();
    }

    //  Command queue is full.
    if (m_pending_commands.size() >= queue_limit){
//        cout << "Command queue is full" << endl;
        return false;
    }

    //  Don't get too far ahead of the oldest seqnum.
    uint64_t seqnum = m_send_seq;
    if (seqnum - oldest_live_seqnum() > MAX_SEQNUM_GAP){
        return false;
    }

    seqnum_t seqnum_s = (seqnum_t)seqnum;
    memcpy(send_params, &seqnum_s, sizeof(seqnum_t));

    std::pair<std::map<uint64_t, PendingCommand>::iterator, bool> ret = m_pending_commands.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(seqnum),
        std::forward_as_tuple()
    );
    if (!ret.second){
        throw "Duplicate sequence number: " + std::to_string(seqnum);
    }

    m_send_seq = seqnum + 1;

    PendingCommand* handle = &ret.first->second;

    handle->silent_remove = silent_remove;
    handle->request = BotBaseMessage(send_type, std::string(send_params, send_bytes));
    handle->first_sent = std::chrono::system_clock::now();

    send_message(handle->request, false);

    iter = ret.first;

    return true;
}
bool PABotBase::issue_request(
    std::map<uint64_t, PendingRequest>::iterator& iter,
    uint8_t send_type, char* send_params, size_t send_bytes,
    bool silent_remove
){
    //  Issue a request or a command and return.
    //
    //  If it cannot be issued (because we're over the limits), this function
    //  will wait until it can be issued.
    //
    //  The "silent_remove" parameter determines what to do when the
    //  ack (for a request) or a finish (for a command) is received.
    //
    //  If (silent_remove = true), the receiving thread will remove the request
    //  from the map and do nothing else. This is for async commands.
    //
    //  If (silent_remove = false), the receiving thread will not remove the
    //  request. Instead, it will notify whatever thread is waiting for the
    //  result. That waiting thread will process the return value (if any) and
    //  remove the request from the map. This is for synchronous commands where
    //  the function waits for the command to finish before returning.
    //

    while (true){
        if (try_issue_request(
            iter,
            send_type, send_params, send_bytes,
            silent_remove, MAX_PENDING_REQUESTS
        )){
            return true;
        }
        std::unique_lock<std::mutex> lg(m_sleep_lock);
        if (m_state.load(std::memory_order_acquire) != State::RUNNING){
            throw CancelledException();
        }
        m_cv.wait(lg);
    }
}
bool PABotBase::issue_command(
    std::map<uint64_t, PendingCommand>::iterator& iter,
    uint8_t send_type, char* send_params, size_t send_bytes,
    bool silent_remove
){
    //  Issue a request or a command and return.
    //
    //  If it cannot be issued (because we're over the limits), this function
    //  will wait until it can be issued.
    //
    //  The "silent_remove" parameter determines what to do when the
    //  ack (for a request) or a finish (for a command) is received.
    //
    //  If (silent_remove = true), the receiving thread will remove the request
    //  from the map and do nothing else. This is for async commands.
    //
    //  If (silent_remove = false), the receiving thread will not remove the
    //  request. Instead, it will notify whatever thread is waiting for the
    //  result. That waiting thread will process the return value (if any) and
    //  remove the request from the map. This is for synchronous commands where
    //  the function waits for the command to finish before returning.
    //

    while (true){
        if (try_issue_command(
            iter,
            send_type, send_params, send_bytes,
            silent_remove, MAX_PENDING_REQUESTS
        )){
            return true;
        }
        std::unique_lock<std::mutex> lg(m_sleep_lock);
        if (m_state.load(std::memory_order_acquire) != State::RUNNING){
            throw CancelledException();
        }
        m_cv.wait(lg);
    }
}


bool PABotBase::try_issue_request(
    uint8_t send_type, char* send_params, size_t send_bytes
){
    if (!PABB_MSG_IS_COMMAND(send_type)){
        std::map<uint64_t, PendingRequest>::iterator iter;
        return try_issue_request(iter, send_type, send_params, send_bytes, true, MAX_PENDING_REQUESTS);
    }else{
        std::map<uint64_t, PendingCommand>::iterator iter;
        return try_issue_command(iter, send_type, send_params, send_bytes, true, MAX_PENDING_REQUESTS);
    }
}
void PABotBase::issue_request(
    uint8_t send_type, char* send_params, size_t send_bytes
){
    if (!PABB_MSG_IS_COMMAND(send_type)){
        std::map<uint64_t, PendingRequest>::iterator iter;
        issue_request(iter, send_type, send_params, send_bytes, true);
    }else{
        std::map<uint64_t, PendingCommand>::iterator iter;
        issue_command(iter, send_type, send_params, send_bytes, true);
    }
}
void PABotBase::issue_request_and_wait(
    uint8_t send_type, char* send_params, size_t send_bytes,
    uint8_t recv_type, char* recv_params, size_t recv_bytes
){
    std::map<uint64_t, PendingRequest>::iterator iter;
    issue_request(iter, send_type, send_params, send_bytes, false);

    const bool is_command = PABB_MSG_IS_COMMAND(send_type);
    AckState end_state = is_command
        ? AckState::FINISHED
        : AckState::ACKED;

    //  Wait for ack.
    while (iter->second.state != end_state){
        std::unique_lock<std::mutex> lg(m_sleep_lock);
        {
            SpinLockGuard slg(m_state_lock, "PABotBase::issue_request_and_wait() - 0");
            State state = m_state.load(std::memory_order_acquire);
            if (state != State::RUNNING){
                remove_request(iter);
                throw CancelledException();
            }
        }
        m_cv.wait(lg);
    }


    //  Verify return payload.
    uint8_t type = iter->second.ack.type;
    if (type != recv_type){
        throw "Received incorrect response type: " + std::to_string(type);
    }
    const std::string& body = iter->second.ack.body;
    if (body.size() != recv_bytes){
        throw "Received incorrect response size: " + std::to_string(body.size());
    }
    memcpy(recv_params, body.c_str(), body.size());
//    remove_request(iter);

    std::unique_lock<std::mutex> lg(m_sleep_lock);
    SpinLockGuard slg(m_state_lock, "PABotBase::issue_request_and_wait() - 1");
    remove_request(iter);
}




uint32_t PABotBase::protocol_version(){
    pabb_MsgRequestProtocolVersion params;
    pabb_MsgAckRequestI32 response;
    issue_request_and_wait<PABB_MSG_REQUEST_PROTOCOL_VERSION, PABB_MSG_ACK_REQUEST_I32>(params, response);
    return response.data;
}
uint32_t PABotBase::program_version(){
    pabb_MsgRequestProgramVersion params;
    pabb_MsgAckRequestI32 response;
    issue_request_and_wait<PABB_MSG_REQUEST_PROGRAM_VERSION, PABB_MSG_ACK_REQUEST_I32>(params, response);
    return response.data;
}
uint8_t PABotBase::program_id(){
    pabb_MsgRequestProgramID params;
    pabb_MsgAckRequestI8 response;
    issue_request_and_wait<PABB_MSG_REQUEST_PROGRAM_ID, PABB_MSG_ACK_REQUEST_I8>(params, response);
    return response.data;
}



}
