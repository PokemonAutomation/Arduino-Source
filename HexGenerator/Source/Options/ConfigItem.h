/*  Config Item
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ConfigItem_H
#define PokemonAutomation_ConfigItem_H

#include <memory>
#include <QJsonValue>
#include <QWidget>

namespace PokemonAutomation{


class ConfigItem{
public:
    static const QString JSON_TYPE;
    static const QString JSON_LABEL;

public:
    ConfigItem(const QJsonObject& obj);
    virtual ~ConfigItem() = default;

    virtual const QString& type() const = 0;
    virtual bool is_valid() const{ return true; }

    virtual void restore_defaults(){}

    virtual QJsonObject to_json() const;
    virtual std::string to_cpp() const{ return ""; }

    virtual QWidget* make_ui(QWidget& parent) = 0;




protected:
    QString m_label;
};



std::unique_ptr<ConfigItem> parse_option(const QJsonObject& obj);

using OptionMaker = std::unique_ptr<ConfigItem> (*)(const QJsonObject& obj);
int register_option(const QString& name, OptionMaker fp);


}
#endif
