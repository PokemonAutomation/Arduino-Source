/*  Pokemon Automation Bot Base - Client Example
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <stdint.h>
#include <string>
#include <chrono>
#include <sstream>
#include <iostream>
#include "Common/MessageProtocol.h"
#include "Common/Cpp/Exception.h"
#include "Common/PokemonSwSh/PokemonSwShMisc.h"
#include "MessageConverter.h"
#include "Logging.h"

namespace PokemonAutomation{

std::string to_string_padded(size_t digits, uint64_t x){
    std::string str = std::to_string(x);
    if (digits < str.size()){
        PA_THROW_StringException("Number is too big to convert to fixed length string.");
    }
    return std::string(digits - str.size(), '0') + str;
}

std::string current_time(){
    //  Based off of: https://stackoverflow.com/questions/15957805/extract-year-month-day-etc-from-stdchronotime-point-in-c

    using namespace std;
    using namespace std::chrono;
    typedef duration<int, ratio_multiply<hours::period, ratio<24> >::type> days;
    system_clock::time_point now = system_clock::now();
    system_clock::duration tp = now.time_since_epoch();
    days d = duration_cast<days>(tp);
    tp -= d;
    hours h = duration_cast<hours>(tp);
    tp -= h;
    minutes m = duration_cast<minutes>(tp);
    tp -= m;
    seconds s = duration_cast<seconds>(tp);
    tp -= s;
    auto micros = 1000000 * tp.count() * system_clock::duration::period::num / system_clock::duration::period::den;
    time_t tt = system_clock::to_time_t(now);
//    tm utc_tm = *gmtime(&tt);
    tm local_tm = *localtime(&tt);

    std::stringstream ss;
    ss << local_tm.tm_year + 1900 << '-';
    ss << to_string_padded(2, local_tm.tm_mon + 1) << '-';
    ss << to_string_padded(2, local_tm.tm_mday) << ' ';
    ss << to_string_padded(2, local_tm.tm_hour) << ':';
    ss << to_string_padded(2, local_tm.tm_min) << ':';
    ss << to_string_padded(2, local_tm.tm_sec) << '.';
    ss << to_string_padded(6, micros);

    return ss.str();
}




std::mutex logging_lock;

void log(const std::stringstream& ss){
    log(ss.str());
}
void log(const std::string& msg){
    std::lock_guard<std::mutex> lg(logging_lock);
    std::cout << current_time() << " - " << msg << std::endl;
}



void MessageLogger::log(std::string msg){
    log(msg);
}
void MessageLogger::on_send(const BotBaseMessage& message, bool is_retransmit){
    bool print = false;
    do{
        if (is_retransmit){
            print = true;
        }
        if (PABB_MSG_IS_REQUEST(message.type)){
            print = true;
        }
        if (PABB_MSG_IS_COMMAND(message.type)){
            print = true;
        }
        if (message.type == PABB_MSG_REQUEST_CLOCK){
            print = false;
        }
        if (message.type == PABB_MSG_CONTROLLER_STATE){
            print = false;
        }

        if (m_log_everything.load(std::memory_order_relaxed)){
            print = true;
        }

    }while (false);
    if (!print){
        return;
    }
    std::string str = message_to_string(message);
    if (str.empty()){
        return;
    }
    if (is_retransmit){
        log("Re-Send: " + str);
    }else{
        log("Sending: " + str);
    }
}
void MessageLogger::on_recv(const BotBaseMessage& message){
    bool print = false;
    do{
        if (PABB_MSG_IS_ERROR(message.type)){
            print = true;
        }
        if (PABB_MSG_IS_INFO(message.type)){
            print = true;
        }

        if (m_log_everything.load(std::memory_order_relaxed)){
            print = true;
        }

    }while (false);
    if (!print){
        return;
    }
    log("Receive: " + message_to_string(message));
}



}

