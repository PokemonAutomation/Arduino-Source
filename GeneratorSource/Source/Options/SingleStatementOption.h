/*  Single-Statement Option
 *
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 *
 */

#ifndef PokemonAutomation_SingleStatementOption_H
#define PokemonAutomation_SingleStatementOption_H

#include <memory>
#include <QJsonValue>
#include <QWidget>
#include "ConfigItem.h"

namespace PokemonAutomation{


class SingleStatementOption : public ConfigItem{
public:
    static const QString JSON_DECLARATION;
    static const QString JSON_DEFAULT;
    static const QString JSON_CURRENT;

public:
    SingleStatementOption(const QJsonObject& obj);

    virtual QJsonObject to_json() const;

protected:
    QString m_declaration;
};


}
#endif
