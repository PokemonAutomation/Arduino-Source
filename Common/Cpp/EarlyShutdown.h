/*  Early Shutdown
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_EarlyShutdown_H
#define PokemonAutomation_EarlyShutdown_H

#include "Logging/AbstractLogger.h"

namespace PokemonAutomation{


template <typename Lambda>
void blocking_shutdown(
    Logger& logger,
    const char* name,
    Lambda try_shutdown
) noexcept{
    try{
        std::string str("Shutting down ");
        str += name;
        str += "...";
        logger.log(str);
        if (try_shutdown()){
            return;
        }
        str = "~";
        str += name;
        str += "(): Failed to shutdown on first attempt. Retrying...";
        logger.log(str, COLOR_RED);
        while (!try_shutdown());
    }catch (...){}
}




}
#endif
