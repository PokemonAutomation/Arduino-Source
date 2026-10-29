/*  Time
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <ctime>
//#include <format>
#include <sstream>
#include "Common/Cpp/PrettyPrint.h"
#include "Time.h"

namespace PokemonAutomation{



std::string current_time_to_str(){
    //  Based off of: https://stackoverflow.com/questions/15957805/extract-year-month-day-etc-from-stdchronotime-point-in-c

    using namespace std;
    using namespace std::chrono;
    typedef duration<int, ratio_multiply<hours::period, ratio<24> >::type> days;
    system_clock::time_point now = system_clock::now();
    system_clock::duration tp = now.time_since_epoch();
    days d = duration_cast<days>(tp);
    tp -= d;
    hours h = duration_cast<hours>(tp);
    tp -= h;
    minutes m = duration_cast<minutes>(tp);
    tp -= m;
    seconds s = duration_cast<seconds>(tp);
    tp -= s;
    auto micros = 1000000 * tp.count() * system_clock::duration::period::num / system_clock::duration::period::den;
    time_t tt = system_clock::to_time_t(now);
//    tm utc_tm = *gmtime(&tt);
    tm local_tm = *localtime(&tt);

    std::ostringstream ss;
    ss << local_tm.tm_year + 1900 << '-';
    ss << tostr_padded(2, local_tm.tm_mon + 1) << '-';
    ss << tostr_padded(2, local_tm.tm_mday) << ' ';
    ss << tostr_padded(2, local_tm.tm_hour) << ':';
    ss << tostr_padded(2, local_tm.tm_min) << ':';
    ss << tostr_padded(2, local_tm.tm_sec) << '.';
    ss << tostr_padded(6, micros);

    return ss.str();
}



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
