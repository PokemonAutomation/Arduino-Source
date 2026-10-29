/*  Time Tools (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QDateTime>
#include <QTimeZone>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "TimeQt.h"

namespace PokemonAutomation{



std::string to_utc_time_str(WallClock time){
#if 0
    time_t tt = std::chrono::system_clock::to_time_t(time);
    static SpinLock lock;
    WriteSpinLock lg(lock);
    std::tm* utc = gmtime(&tt);
    std::string str;
    str += std::to_string(utc->tm_year + 1900);
    str += "-" + tostr_padded(2, utc->tm_mon + 1);
    str += "-" + tostr_padded(2, utc->tm_mday);
    str += " " + tostr_padded(2, utc->tm_hour);
    str += ":" + tostr_padded(2, utc->tm_min);
    str += ":" + tostr_padded(2, utc->tm_sec);
    return str;
#endif
#if 0
    QDateTime now = QDateTime::fromStdTimePoint(time);

    QDate qdate = now.date();
    QTime qtime = now.time();
    std::string str;
    str += std::to_string(qdate.year());
    str += "-" + tostr_padded(2, qdate.month());
    str += "-" + tostr_padded(2, qdate.day());
    str += " " + tostr_padded(2, qtime.hour());
    str += ":" + tostr_padded(2, qtime.minute());
    str += ":" + tostr_padded(2, qtime.second());
    return str;

#endif

    int64_t secs_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count();
    QDateTime qdatetime = QDateTime::fromSecsSinceEpoch(secs_since_epoch, QTimeZone::utc());
    QDate qdate = qdatetime.date();
    QTime qtime = qdatetime.time();
    std::string str;
    str += std::to_string(qdate.year());
    str += "-" + tostr_padded(2, qdate.month());
    str += "-" + tostr_padded(2, qdate.day());
    str += " " + tostr_padded(2, qtime.hour());
    str += ":" + tostr_padded(2, qtime.minute());
    str += ":" + tostr_padded(2, qtime.second());
    return str;
}
WallClock parse_utc_time_str(const std::string& str){
    int year = std::atoi(&str[0]);
    if (year <= 1900){
        throw ParseException("Invalid date format: Year must be greater than 1900.");
    }
//    cout << year << endl;

    auto index = str.find("-");
    if (index == std::string::npos){
        throw ParseException("Invalid date format: Expected hyphen.");
    }
    int month = std::atoi(&str[++index]);
//    cout << month << endl;
    if (month < 1 || month > 12){
        throw ParseException("Invalid date format: Month out of range.");
    }

    index = str.find("-", index);
    if (index == std::string::npos){
        throw ParseException("Invalid date format: Expected hyphen.");
    }
    int day = std::atoi(&str[++index]);
    if (day < 1 || day > 31){
        throw ParseException("Invalid date format: Day out of range.");
    }
//    cout << day << endl;

    index = str.find(" ", index);
    if (index == std::string::npos){
        throw ParseException("Invalid date format: Expected space.");
    }
    int hour = std::atoi(&str[++index]);
    if (hour < 0 || hour > 23){
        throw ParseException("Invalid date format: Hour out of range.");
    }
//    cout << hour << endl;

    index = str.find(":", index);
    if (index == std::string::npos){
        throw ParseException("Invalid date format: Expected colon.");
    }
    int minutes = std::atoi(&str[++index]);
    if (minutes < 0 || minutes > 60){
        throw ParseException("Invalid date format: Minutes out of range.");
    }
//    cout << minutes << endl;

    index = str.find(":", index);
    if (index == std::string::npos){
        throw ParseException("Invalid date format: Expected colon.");
    }
    int seconds = std::atoi(&str[++index]);
    if (seconds < 0 || seconds > 61){
        throw ParseException("Invalid date format: Seconds out of range.");
    }
//    cout << seconds << endl;

#if 0
    std::tm tm{};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = minutes;
    tm.tm_sec = seconds;
    std::time_t t = std::mktime(&tm);
//    std::tm* tm1 = std::localtime(&t);
    if (t == -1){
        throw ParseException("Invalid date.");
    }
#endif

//    std::chrono::utc_clock::fr

//    WallClock utc = std::chrono::system_clock::from_time_t(t);

    QDate qdate(year, month, day);
    if (!qdate.isValid()){
        throw ParseException("Invalid date.");
    }
    QTime qtime(hour, minutes, seconds);
    if (!qtime.isValid()){
        throw ParseException("Invalid time.");
    }

    QDateTime qdatetime(qdate, qtime, QTimeZone::utc());
    int64_t secs_since_epoch = qdatetime.toSecsSinceEpoch();

    return WallClock{} + std::chrono::seconds(secs_since_epoch);
}



int64_t to_seconds_since_epoch(const DateTime& date){
    QDate qdate(date.year, date.month, date.day);
    QTime qtime(date.hour, date.minute, date.second);
    QDateTime qdatetime(qdate, qtime, QTimeZone::utc());
    int64_t secs_since_epoch = qdatetime.toSecsSinceEpoch();
    return secs_since_epoch;
}
DateTime from_seconds_since_epoch(int64_t seconds_since_epoch){
    QDateTime qdatetime = QDateTime::fromSecsSinceEpoch(seconds_since_epoch, QTimeZone::utc());
    QDate qdate = qdatetime.date();
    QTime qtime = qdatetime.time();
    return DateTime{
        (int16_t)qdate.year(),
        (int8_t)qdate.month(),
        (int8_t)qdate.day(),
        (int8_t)qtime.hour(),
        (int8_t)qtime.minute(),
        (int8_t)qtime.second(),
    };
}





}
