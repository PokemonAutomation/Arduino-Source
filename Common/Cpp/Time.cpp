/*  Time
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <ctime>
//#include <format>
#include "Time.h"

namespace PokemonAutomation{



uint16_t current_year(){
#if 0
    //  This requires C++20. (https://stackoverflow.com/a/58152002)
    return (uint16_t)std::atoi(
        std::format("{:%Y}", std::chrono::system_clock::now())
    );
#endif

#if 1
    //  This is not thread-safe. (https://stackoverflow.com/a/58153628)
    //  (Thread-safe on Windows because it uses TLS, but unclear on other platforms.)
    std::time_t t = std::time(nullptr);
    std::tm* info = std::localtime(&t);
    return (uint16_t)(info->tm_year + 1900);
#endif

#if 0
    //  This is wrong. (https://stackoverflow.com/a/67459754)
    time_t current_time = time(nullptr);
    return (uint16_t)(1970 + current_time / 31537970);
#endif
}



}
