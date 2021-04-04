/*  Qt JSON Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_QtJsonTools_H
#define PokemonAutomation_QtJsonTools_H

#include <memory>
#include <QDate>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

namespace PokemonAutomation{


QJsonDocument read_json_file(const QString& path);
void write_json_file(const QString& path, const QJsonDocument& json);

QJsonValue json_get_value_throw(const QJsonObject& obj, const QString& key);
bool json_get_bool_throw(const QJsonObject& obj, const QString& key);
int json_get_int_throw(const QJsonObject& obj, const QString& key);
double json_get_double_throw(const QJsonObject& obj, const QString& key);
QString json_get_string_throw(const QJsonObject& obj, const QString& key);
QJsonArray json_get_array_throw(const QJsonObject& obj, const QString& key);
QJsonObject json_get_object_throw(const QJsonObject& obj, const QString& key);

QJsonValue json_get_value_nothrow(const QJsonObject& obj, const QString& key);
QJsonArray json_get_array_nothrow(const QJsonObject& obj, const QString& key);
QJsonObject json_get_object_nothrow(const QJsonObject& obj, const QString& key);

bool valid_switch_date(const QDate& date);
QJsonArray json_write_date(const QDate& date);


template <typename Destination>
bool json_get_bool(Destination& destination, const QJsonObject& obj, const QString& key){
    auto iter = obj.find(key);
    if (iter == obj.end() || !iter->isBool()){
        return false;
    }
    destination = iter->toBool();
    return true;
}
template <typename Destination>
bool json_get_int(
    Destination& destination, const QJsonObject& obj, const QString& key,
    int min_value = std::numeric_limits<int>::min(),
    int max_value = std::numeric_limits<int>::max()
){
    auto iter = obj.find(key);
    if (iter == obj.end() || !iter->isDouble()){
        return false;
    }
    int x = iter->toInt();
    if (x < min_value){
        x = min_value;
    }
    if (x > max_value){
        x = max_value;
    }
    destination = x;
    return true;
}
template <typename Destination>
bool json_get_string(Destination& destination, const QJsonObject& obj, const QString& key){
    auto iter = obj.find(key);
    if (iter == obj.end() || !iter->isString()){
        return false;
    }
    destination = iter->toString();
    return true;
}


template <typename Destination>
bool json_parse_date(Destination& date, const QJsonValue& value){
    QJsonArray array = value.toArray();
    if (array.size() != 3){
        return false;
    }
    for (int c = 0; c < 3; c++){
        if (!array[c].isDouble()){
            return false;
        }
    }
    int year = array[0].toInt();
    int month = array[1].toInt();
    int day = array[2].toInt();
    QDate try_date(year, month, day);
    if (!try_date.isValid() || !valid_switch_date(try_date)){
        return false;
    }
    date = try_date;
    return true;
}


}
#endif
