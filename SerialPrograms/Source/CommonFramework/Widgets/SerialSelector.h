/*  Serial Connection Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SerialSelector_H
#define PokemonAutomation_SerialSelector_H

#include <QSerialPortInfo>
#include "Common/PokemonSwSh/PokemonProgramIDs.h"
#include "CommonFramework/Logging/DeviceLogger.h"

class QJsonValue;
class QWidget;

namespace PokemonAutomation{


class SerialSelectorWidget;


class SerialSelector{
public:
    SerialSelector(
        QString label, std::string logger_tag,
        PABotBaseLevel minimum_pabotbase
    );
    SerialSelector(
        QString label, std::string logger_tag,
        PABotBaseLevel minimum_pabotbase,
        const QJsonValue& json
    );

    void load_json(const QJsonValue& json);
    QJsonValue to_json() const;

    const QSerialPortInfo* port() const;

    SerialSelectorWidget* make_ui(QWidget& parent, Logger& logger);

private:
    friend class SerialSelectorWidget;

    const QString m_label;
    const PABotBaseLevel m_minimum_pabotbase;
    const std::string m_logger_tag;

    QSerialPortInfo m_port;
};





}
#endif


