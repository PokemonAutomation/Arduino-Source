/*  Serial Port Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SerialPortOption_H
#define PokemonAutomation_SerialPortOption_H

#include <memory>
#include "Common/Cpp/Pimpl.h"
#include "Common/PokemonSwSh/PokemonProgramIDs.h"
#include "CommonFramework/Logging/LoggerQt.h"

class QSerialPortInfo;

namespace PokemonAutomation{

class JsonValue;
class SerialPortWidget;


class SerialPortOption{
public:
    ~SerialPortOption();
    SerialPortOption(PABotBaseLevel minimum_pabotbase);

    void load_json(const JsonValue& json);
    JsonValue to_json() const;

    const QSerialPortInfo* port() const;

private:
    friend class SerialPortSession;
    friend class SerialPortWidget;

    const PABotBaseLevel m_minimum_pabotbase;

    Pimpl<QSerialPortInfo> m_port;
};





}
#endif


