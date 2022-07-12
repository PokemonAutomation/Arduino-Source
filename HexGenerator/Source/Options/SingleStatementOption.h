/*  Single-Statement Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SingleStatementOption_H
#define PokemonAutomation_SingleStatementOption_H

#include <memory>
#include <QWidget>
#include "ConfigItem.h"

namespace PokemonAutomation{


class SingleStatementOption : public ConfigItem{
public:
    static const std::string JSON_DECLARATION;
    static const std::string JSON_DEFAULT;
    static const std::string JSON_CURRENT;

public:
    SingleStatementOption(const JsonObject& obj);

    virtual JsonObject to_json() const;

protected:
    std::string m_declaration;
};


}
#endif
