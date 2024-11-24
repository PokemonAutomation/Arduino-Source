/*  OS Sleep (Windows)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <mutex>
#include <Windows.h>
#include "Common/Qt/Redispatch.h"
#include "CommonFramework/Logging/Logger.h"
#include "SystemSleep.h"

namespace PokemonAutomation{


struct SystemSleepController::InternalController{
    std::mutex lock;
    size_t screen_on_requests = 0;
    size_t no_sleep_requests = 0;

    void update_state(){
        //  SetThreadExecutionState(ES_CONTINUOUS) only lasts as long as the
        //  thread is alive. So we redispatch to the main thread.
        queue_on_main_thread([
            screen_on_requests = screen_on_requests,
            no_sleep_requests = no_sleep_requests
        ]{
            EXECUTION_STATE flags = ES_CONTINUOUS;
            if (screen_on_requests > 0){
                flags |= ES_DISPLAY_REQUIRED;
            }
            if (no_sleep_requests > 0){
                flags |= ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED;
            }
            EXECUTION_STATE ret = SetThreadExecutionState(flags);
            if (ret == 0){
                global_logger_tagged().log("Unable to set system sleep state.", COLOR_RED);
            }else if (screen_on_requests > 0){
                global_logger_tagged().log("Setting sleep state to: Screen On", COLOR_BLUE);
            }else if (no_sleep_requests > 0){
                global_logger_tagged().log("Setting sleep state to: No Sleep", COLOR_BLUE);
            }else{
                global_logger_tagged().log("Setting sleep state to: None", COLOR_BLUE);
            }
        });
    }
};






}
