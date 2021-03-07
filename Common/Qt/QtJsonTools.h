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

namespace PokemonAutomation{


QJsonDocument read_json_file(const QString& path);
void write_json_file(const QString& path, const QJsonDocument& json);

int json_cast_int(const QJsonValue& value);
QString json_cast_string(const QJsonValue& value);
QJsonArray json_cast_array(const QJsonValue& value);
QJsonObject json_cast_object(const QJsonValue& value);

QJsonValue json_get_value(const QJsonObject& obj, const QString& key);
bool json_get_bool(const QJsonObject& obj, const QString& key);
int json_get_int(const QJsonObject& obj, const QString& key);
double json_get_double(const QJsonObject& obj, const QString& key);
QString json_get_string(const QJsonObject& obj, const QString& key);
QJsonArray json_get_array(const QJsonObject& obj, const QString& key);
QJsonObject json_get_object(const QJsonObject& obj, const QString& key);

bool valid_switch_date(const QDate& date);
QJsonArray json_write_date(const QDate& date);
QDate json_parse_date(const QJsonValue& value);
QDate json_get_date(const QJsonObject& obj, const QString& key);


}
#endif
