/*  Global Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "MultiOutputLogger.h"
#include "LastLogTracker.h"
#include "GlobalLogger.h"

namespace PokemonAutomation{


Logger& global_logger_raw(){
    return global_multi_logger();
}

LastLogTracker& global_last_log_history(){
    static LastLogTracker tracker;
    return tracker;
}

MultiOutputLogger& global_multi_logger(){
    static MultiOutputLogger logger;
    return logger;
}



}
