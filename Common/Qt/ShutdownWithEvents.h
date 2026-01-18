/*  Shutdown with Events
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ShutdownWithEvents_H
#define PokemonAutomation_ShutdownWithEvents_H

#include <QApplication>
#include "Common/Cpp/Logging/AbstractLogger.h"

namespace PokemonAutomation{


template <typename Lambda>
void shutdown_with_events(
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
        do{
            QApplication::processEvents();
        }while (!try_shutdown());
    }catch (...){}
}



}
#endif
