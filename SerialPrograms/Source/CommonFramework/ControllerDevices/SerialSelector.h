/*  Serial Connection Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SerialSelector_H
#define PokemonAutomation_SerialSelector_H

#include <memory>
#include "Common/Cpp/Pimpl.h"
#include "Common/PokemonSwSh/PokemonProgramIDs.h"
#include "CommonFramework/Logging/LoggerQt.h"

class QSerialPortInfo;
class QWidget;

namespace PokemonAutomation{

class JsonValue;
class SerialSelectorWidget;


class SerialSelector{
public:
    ~SerialSelector();
    SerialSelector(PABotBaseLevel minimum_pabotbase);

    void load_json(const JsonValue& json);
    JsonValue to_json() const;

    const QSerialPortInfo* port() const;

    SerialSelectorWidget* make_ui(QWidget& parent, LoggerQt& logger);

private:
    friend class SerialSelectorWidget;

    const PABotBaseLevel m_minimum_pabotbase;

    Pimpl<QSerialPortInfo> m_port;
};





}
#endif


