/*  Environment (Linux)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#if defined(__linux) || defined(__APPLE__)

#include <time.h>
#include <set>
#include <iostream>
#include <thread>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Logging/Logger.h"
#include "Environment.h"

#if defined(__APPLE__)
#include <libproc.h>
#endif

// #include <iostream>


namespace PokemonAutomation{




const EnumDropdownDatabase<ThreadPriority>& PRIORITY_DATABASE(){
    static EnumDropdownDatabase<ThreadPriority> database({
        {ThreadPriority::Max, "max", "Max Priority"},
        {ThreadPriority::Min, "min", "Min Priority"},
    });
    return database;
}

bool set_thread_priority(Logger& logger, ThreadPriority priority){
    int native_priority = sched_get_priority_min(SCHED_RR);
    switch (priority){
    case ThreadPriority::Max:
        native_priority = sched_get_priority_max(SCHED_RR);
        break;
    case ThreadPriority::Min:
        native_priority = sched_get_priority_min(SCHED_RR);
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Priority Index: " + std::to_string((int)priority));
    }

    struct sched_param param;
    param.sched_priority = native_priority;
    if (pthread_setschedparam(pthread_self(), SCHED_RR, &param) == 0){
        logger.log("Thread priority set to: " + PRIORITY_DATABASE().find(priority)->display, COLOR_BLUE);
        return true;
    }

    int errorcode = errno;
    logger.log("Unable to set process priority. Error Code = " + std::to_string(errorcode), COLOR_RED);
    return false;
}












}
#endif
