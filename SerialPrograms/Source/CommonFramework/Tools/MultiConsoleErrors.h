/*  Multi-Console Errors
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_MultiConsoleErrors_H
#define PokemonAutomation_CommonFramework_MultiConsoleErrors_H

#include <string>
#include <atomic>

namespace PokemonAutomation{

class Logger;


class MultiConsoleErrorState{
public:
    void report_unrecoverable_error(Logger& logger, const std::string& msg);
    void check_unrecoverable_error(Logger& logger);

private:
    std::atomic<bool> m_unrecoverable_error;
};




}
#endif
