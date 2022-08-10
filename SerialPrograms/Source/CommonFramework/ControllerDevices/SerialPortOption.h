/*  Serial Port Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SerialPortOption_H
#define PokemonAutomation_SerialPortOption_H

#include <memory>
#include "Common/Cpp/Pimpl.h"
#include "CommonFramework/ControllerDevices/SerialPortGlobals.h"

class QSerialPortInfo;

namespace PokemonAutomation{

class JsonValue;
class SerialPortWidget;


class SerialPortOption{
public:
    ~SerialPortOption();
    SerialPortOption(PABotBaseLevel minimum_pabotbase);

    void clear();

    void load_json(const JsonValue& json);
    JsonValue to_json() const;


public:
    PABotBaseLevel minimum_pabotbase() const{ return m_minimum_pabotbase; }

    const QSerialPortInfo* port() const;
    void set_port(QSerialPortInfo port);


private:
    const PABotBaseLevel m_minimum_pabotbase;

    Pimpl<QSerialPortInfo> m_port;
};





}
#endif


