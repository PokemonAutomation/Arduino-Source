/*  Parent Class for all Configuration Files
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ConfigSet_H
#define PokemonAutomation_ConfigSet_H

#include <QObject>
#include "RightPanel.h"

namespace PokemonAutomation{

class JsonArray;
class JsonObject;


class ConfigSet : public RightPanel{
public:
    static const std::string JSON_CONFIG_NAME;
    static const std::string JSON_CONFIG_PATH;
    static const std::string JSON_DOCUMENTATION;
    static const std::string JSON_DESCRIPTION;
    static const std::string JSON_HEADER;
    static const std::string JSON_OPTIONS;

public:
    ConfigSet(std::string category, const JsonObject& obj);

    const std::string& description() const{ return m_description; }

    //  Returns error message if invalid. Otherwise returns empty string.
    virtual QString check_validity() const{ return QString(); }

    virtual void restore_defaults(){}

    JsonObject to_json() const;
    std::string to_cfile() const;

    std::string save_json() const;
    std::string save_cfile() const;

    virtual QWidget* make_ui(MainWindow& parent) override;

protected:
    virtual QWidget* make_options_body(QWidget& parent) = 0;
    virtual JsonArray options_json() const;
    virtual std::string options_cpp() const{ return ""; }

private:
    std::string m_path;
    std::string m_description;
    std::string m_header;
};


}
#endif
