/*  Global Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "MultiOutputLogger.h"
#include "GlobalLogger.h"

namespace PokemonAutomation{


Logger& global_logger_raw(){
    return global_multi_logger();
}
MultiOutputLogger& global_multi_logger(){
    static MultiOutputLogger logger;
    return logger;
}



}
