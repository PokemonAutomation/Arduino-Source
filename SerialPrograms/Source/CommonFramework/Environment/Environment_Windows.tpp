/*  Environment (Windows)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef _WIN32

#include <map>
#include <iostream>
#include <thread>
#include <Windows.h>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Logging/Logger.h"
#include "Environment.h"
#include "Environment_Windows.h"

#if __GNUC__
#ifndef cpuid_H
#define cpuid_H
#include <cpuid.h>
#endif
#endif

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{




const EnumDropdownDatabase<ThreadPriority>& PRIORITY_DATABASE(){
    static EnumDropdownDatabase<ThreadPriority> database({
        {ThreadPriority::Realtime,      "realtime",     "Realtime"},
        {ThreadPriority::High,          "high",         "High"},
        {ThreadPriority::AboveNormal,   "above-normal", "Above Normal"},
        {ThreadPriority::Normal,        "normal",       "Normal"},
        {ThreadPriority::BelowNormal,   "below-normal", "Below Normal"},
        {ThreadPriority::Low,           "low",          "Low"},
    });
    return database;
}

bool set_thread_priority(Logger& logger, ThreadPriority priority){
    int native_priority;
    switch (priority){
    case ThreadPriority::Realtime:
        native_priority = THREAD_PRIORITY_TIME_CRITICAL;
        break;
    case ThreadPriority::High:
        native_priority = THREAD_PRIORITY_HIGHEST;
        break;
    case ThreadPriority::AboveNormal:
        native_priority = THREAD_PRIORITY_ABOVE_NORMAL;
        break;
    case ThreadPriority::Normal:
        native_priority = THREAD_PRIORITY_NORMAL;
        break;
    case ThreadPriority::BelowNormal:
        native_priority = THREAD_PRIORITY_BELOW_NORMAL;
        break;
    case ThreadPriority::Low:
        native_priority = THREAD_PRIORITY_IDLE;
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Priority: " + std::to_string((int)priority));
    }
    if (SetThreadPriority(GetCurrentThread(), native_priority)){
//        cout << "Thread priority set to: " + PRIORITY_DATABASE().find(priority)->display << endl;
        logger.log("Thread priority set to: " + PRIORITY_DATABASE().find(priority)->display, COLOR_BLUE);
        return true;
    }
    DWORD error = GetLastError();
    logger.log("Unable to set thread priority. Error Code = " + std::to_string(error), COLOR_RED);
    return false;
}





















}
#endif
