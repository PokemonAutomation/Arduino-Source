/*  Multi-Console Errors
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_MultiConsoleErrors_H
#define PokemonAutomation_CommonTools_MultiConsoleErrors_H

#include <string>
#include <atomic>

namespace PokemonAutomation{

class Logger;
class VideoStream;


class MultiConsoleErrorState{
public:
    void report_unrecoverable_error(VideoStream& stream, std::string msg);
    void check_unrecoverable_error(Logger& logger);

private:
    std::atomic<bool> m_unrecoverable_error;
    std::string m_message;
};




}
#endif
