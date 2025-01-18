/*  Serial Port (PABotBase) Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Controllers_SerialPABotBase_Option_H
#define PokemonAutomation_Controllers_SerialPABotBase_Option_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "SerialPABotBase_Globals.h"

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
    void set_port(const QSerialPortInfo* port);


private:
    const PABotBaseLevel m_minimum_pabotbase;

    Pimpl<QSerialPortInfo> m_port;
};





}
#endif


