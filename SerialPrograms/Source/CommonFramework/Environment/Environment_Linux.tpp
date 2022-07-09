/*  Environment (Linux)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#if defined(__linux) || defined(__APPLE__)

#include <set>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "Environment.h"

namespace PokemonAutomation{



extern const int DEFAULT_PRIORITY_INDEX = 1;
const std::vector<std::string> PRIORITY_MODES{
    "Max Priority",
    "Min Priority",
};
int priority_name_to_index(const std::string& name){
    for (size_t c = 0; c < PRIORITY_MODES.size(); c++){
        if (name == PRIORITY_MODES[c]){
            return (int)c;
        }
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Priority String: " + name);
}
bool set_priority_by_index(int index){
    int native_priority = sched_get_priority_min(SCHED_RR);
    switch (index){
    case 0:
        native_priority = sched_get_priority_max(SCHED_RR);
        break;
    case 1:
        native_priority = sched_get_priority_min(SCHED_RR);
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Priority Index: " + std::to_string(index));
    }

    struct sched_param param;
    param.sched_priority = native_priority;
    if (pthread_setschedparam(pthread_self(), SCHED_RR, &param) == 0){
        return true;
    }

    int errorcode = errno;
    global_logger_tagged().log("Unable to set process priority. Error Code = " + std::to_string(errorcode), COLOR_RED);
    return false;
}
int read_priority_index(){
    return -1;
}



extern const int THREAD_PRIORITY_MIN = 0;
extern const int THREAD_PRIORITY_MAX = 1;
const char* thread_priority_name(int priority){
    switch (priority){
    case 0:
        return "Min Priority";
    case 1:
        return "Max Priority";
    default:
        return nullptr;
    }
}
bool set_thread_priority(int priority){
    int native_priority = priority < 0
        ? sched_get_priority_min(SCHED_RR)
        : sched_get_priority_max(SCHED_RR);

    struct sched_param param;
    param.sched_priority = native_priority;

    if (pthread_setschedparam(pthread_self(), SCHED_RR, &param) == 0){
        return true;
    }

    int errorcode = errno;
    std::cerr << "Unable to set thread priority. Error Code = " << errorcode << std::endl;
    return false;
}
QThread::Priority to_qt_priority(int priority){
    if (priority < 0){
        return QThread::LowPriority;
    }else{
        return QThread::HighPriority;
    }
}





































}
#endif
