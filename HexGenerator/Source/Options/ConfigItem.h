/*  Config Item
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ConfigItem_H
#define PokemonAutomation_ConfigItem_H

#include <memory>
#include <QWidget>

namespace PokemonAutomation{

class JsonObject;


class ConfigItem{
public:
    static const std::string JSON_TYPE;
    static const std::string JSON_LABEL;

public:
    ConfigItem(const JsonObject& obj);
    virtual ~ConfigItem() = default;

    virtual const std::string& type() const = 0;

    //  Returns error message if invalid. Otherwise returns empty string.
    virtual std::string check_validity() const{ return std::string(); }

    virtual void restore_defaults(){}

    virtual JsonObject to_json() const;
    virtual std::string to_cpp() const{ return ""; }

    virtual QWidget* make_ui(QWidget& parent) = 0;




protected:
    std::string m_label;
};



std::unique_ptr<ConfigItem> parse_option(const JsonObject& obj);

using OptionMaker = std::unique_ptr<ConfigItem> (*)(const JsonObject& obj);
int register_option(const std::string& name, OptionMaker fp);


}
#endif
