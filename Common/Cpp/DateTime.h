/*  DateTime
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_DateTime_H
#define PokemonAutomation_DateTime_H

#include <stdint.h>

namespace PokemonAutomation{


struct DateTime{
    //  Negative means invalid or cannot read.
    int16_t year = -1;
    int8_t month = -1;
    int8_t day = -1;
    int8_t hour = -1;
    int8_t minute = -1;
    int8_t second = -1;

    bool operator<(const DateTime& x) const{
        if (year != x.year) return year < x.year;
        if (month != x.month) return month < x.month;
        if (day != x.day) return day < x.day;
        if (hour != x.hour) return hour < x.hour;
        if (minute != x.minute) return minute < x.minute;
        if (second != x.second) return second < x.second;
        return false;
    }
    bool operator>(const DateTime& x) const{
        return x < *this;
    }
    bool operator==(const DateTime& x) const{
        return year == x.year
            && month == x.month
            && day == x.day
            && hour == x.hour
            && minute == x.minute
            && second == x.second;
    }
};


}
#endif
