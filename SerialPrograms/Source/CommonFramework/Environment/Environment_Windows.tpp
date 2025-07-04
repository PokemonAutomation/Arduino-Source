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
    DWORD native_priority;
    switch (priority){
    case ThreadPriority::Realtime:
        native_priority = REALTIME_PRIORITY_CLASS;
        break;
    case ThreadPriority::High:
        native_priority = HIGH_PRIORITY_CLASS;
        break;
    case ThreadPriority::AboveNormal:
        native_priority = ABOVE_NORMAL_PRIORITY_CLASS;
        break;
    case ThreadPriority::Normal:
        native_priority = NORMAL_PRIORITY_CLASS;
        break;
    case ThreadPriority::BelowNormal:
        native_priority = BELOW_NORMAL_PRIORITY_CLASS;
        break;
    case ThreadPriority::Low:
        native_priority = IDLE_PRIORITY_CLASS;
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Priority: " + std::to_string((int)priority));
    }
    if (SetPriorityClass(GetCurrentThread(), native_priority)){
//        cout << "Thread priority set to: " + PRIORITY_DATABASE().find(priority)->display << endl;
        logger.log("Thread priority set to: " + PRIORITY_DATABASE().find(priority)->display, COLOR_BLUE);
        return true;
    }
    DWORD error = GetLastError();
    logger.log("Unable to set process priority. Error Code = " + std::to_string(error), COLOR_RED);
    return false;
}





















}
#endif
