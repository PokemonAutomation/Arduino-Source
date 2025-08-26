/*  Pretty Printing
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#include <stdio.h>
#include <chrono>
#include <sstream>
#include <iomanip>
#include "Exceptions.h"
#include "PrettyPrint.h"

namespace PokemonAutomation{



std::string tostr_padded(size_t digits, uint64_t x){
    std::string str = std::to_string(x);
    if (digits < str.size()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Number is too big to convert to fixed length string.");
    }
    return std::string(digits - str.size(), '0') + str;
}
std::string tostr_u_commas(int64_t x){
    //  Prints out x with comma separators.

    std::string str = std::to_string(x);
    std::string out;

    const char* ptr = str.c_str();
    // len: how many digits, don't count "-" in the negative numbers
    size_t len = str.size() - (x < 0);

    size_t commas = (len + 2) / 3 - 1;
    size_t shift = len - commas * 3 + (x < 0);

    while (true){
        char ch = *ptr++;
        if (ch == '\0')
            break;
        if (shift == 0){
            out += ',';
            shift = 3;
        }
        out += ch;
        shift--;
    }

    return out;
}


inline std::string byte_prefix(size_t index){
    static const char* BYTE_NAMES[] = {
        " bytes",
        " KiB",
        " MiB",
        " GiB",
        " TiB",
        " PiB",
        " EiB",
        " ZiB",
        " YiB",
    };
    constexpr size_t SUFFIX_SIZE = sizeof(BYTE_NAMES) / sizeof(const char*);

    if (index < SUFFIX_SIZE){
        return BYTE_NAMES[index];
    }

    return " * 2^" + std::to_string(index) + "0";
}

template <typename IntegerType>
std::string tostr_ui_bytes(IntegerType bytes){
    //  Prints out bytes in one of the following forms:
    //  0.xx suffix
    //  x.xx suffix
    //  xx.x suffix
    //   xxx suffix

    //  Conditions:
    //      "bytes" must be non-negative

    std::string out;
    if (bytes < 1000){
        //  Downcast to a more efficient type.
        uint32_t sbytes = (uint32_t)bytes;
        if (bytes < 10){
//            out += " ";
        }
        out += std::to_string(sbytes);
        out += byte_prefix(0);
        return out;
    }

    size_t suffix_index = 1;
    while (bytes >= 1024000){
        bytes >>= 10;
        suffix_index++;
    }

    //  Downcast to a more efficient type.
    uint32_t sbytes = (uint32_t)bytes;

    sbytes *= 1000;
    sbytes >>= 10;

    //  .xxx
    if (sbytes < 995){
        sbytes += 5;
        sbytes /= 10;

        out += "0.";
        out += std::to_string(sbytes);
        out += byte_prefix(suffix_index);
        return out;
    }

    //  x.xx
    if (sbytes < 9995){
        sbytes += 5;
        sbytes /= 10;

        out += std::to_string(sbytes / 100);
        sbytes %= 100;
        out += ".";
        if (sbytes >= 10){
            out += std::to_string(sbytes);
        }else{
            out += "0";
            out += std::to_string(sbytes);
        }
        out += byte_prefix(suffix_index);
        return out;
    }

    //  xx.x or (0.98)
    if (sbytes < 99950){
        sbytes += 50;
        sbytes /= 100;

        out += std::to_string(sbytes / 10);
        sbytes %= 10;
        out += ".";
        out += std::to_string(sbytes);
        out += byte_prefix(suffix_index);
        return out;
    }

    //  xxx or (1.00)
    {
        sbytes += 500;
        sbytes /= 1000;

//        out += " ";
        out += std::to_string(sbytes);
        out += byte_prefix(suffix_index);
        return out;
    }
}
std::string tostr_bytes(uint64_t bytes){
    return tostr_ui_bytes(bytes);
}



std::string tostr_default(double x){
    std::ostringstream ss;
    ss << x;
    return ss.str();
}
std::string tostr_fixed(double x, int precision){
    std::ostringstream out;
    out << std::setprecision(precision);
    out << std::fixed;
    out << x;
    return out.str();
}


std::string now_to_filestring(){
#if _WIN32 && _MSC_VER
#pragma warning(disable:4996)
#endif

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

    std::stringstream ss;
    ss << local_tm.tm_year + 1900;
    ss << tostr_padded(2, local_tm.tm_mon + 1);
    ss << tostr_padded(2, local_tm.tm_mday) << '-';
    ss << tostr_padded(2, local_tm.tm_hour);
    ss << tostr_padded(2, local_tm.tm_min);
    ss << tostr_padded(2, local_tm.tm_sec);
    ss << tostr_padded(6, micros);

    return ss.str();
}

std::string set_to_str(const std::set<std::string>& set){
    std::string str = "{";
    bool first = true;
    for (const std::string& item : set){
        if (!first){
            str += ", ";
        }
        first = false;
        str += "\"";
        str += item;
        str += "\"";
    }
    str += "}";
    return str;
}

std::string duration_to_string(std::chrono::milliseconds milliseconds){
    const uint64_t SECOND = 1000;
    const uint64_t MINUTE = SECOND * 60;
    const uint64_t HOUR = MINUTE * 60;
    const uint64_t DAY = HOUR * 24;
    const uint64_t WEEK = DAY * 7;
    const uint64_t YEARS = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::years(1)).count();
    const uint64_t MILLION_YEARS = YEARS * 1000000;

    uint64_t ticks = milliseconds.count();

    std::string str;
    if (ticks < MINUTE * 2){
        str += tostr_fixed((double)ticks / SECOND, 3);
        str += " seconds";
    }else if (ticks < HOUR * 2){
        str += tostr_fixed((double)ticks / MINUTE, 3);
        str += " minutes";
    }else if (ticks < DAY * 2){
        str += tostr_fixed((double)ticks / HOUR, 3);
        str += " hours";
    }else if (ticks < WEEK * 2){
        str += tostr_fixed((double)ticks / DAY, 3);
        str += " days";
    }else if (ticks < YEARS * 2){
        str += tostr_fixed((double)ticks / WEEK, 3);
        str += " weeks";
    }else if (ticks < MILLION_YEARS){
        str += tostr_fixed((double)ticks / YEARS, 3);
        str += " years";
    }else{
        str += tostr_fixed((double)ticks / MILLION_YEARS, 3);
        str += " million years";
    }
    return str;
}


// After C++20 format or fmt::format is adopted, we can use:
// std::format("{:x}", value));
std::string tostr_hex(uint64_t x){
    std::ostringstream ss;
    ss << std::hex << x;
    return ss.str();
}






}
