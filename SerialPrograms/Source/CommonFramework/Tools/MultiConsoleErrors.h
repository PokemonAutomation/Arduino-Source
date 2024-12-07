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
class ConsoleHandle;


class MultiConsoleErrorState{
public:
    void report_unrecoverable_error(ConsoleHandle& console, std::string msg);
    void check_unrecoverable_error(Logger& logger);

private:
    std::atomic<bool> m_unrecoverable_error;
    std::string m_message;
};




}
#endif
