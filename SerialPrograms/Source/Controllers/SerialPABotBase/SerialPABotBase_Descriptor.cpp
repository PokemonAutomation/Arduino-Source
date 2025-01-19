/*  Serial Port (PABotBase) Descriptor
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "SerialPABotBase_Descriptor.h"
#include "SerialPABotBase_Connection.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace SerialPABotBase{


const char SerialDescriptor::TYPENAME[] = "SerialPort";


std::unique_ptr<ControllerDescriptor> SerialDescriptor::clone() const{
    return std::unique_ptr<ControllerDescriptor>(new SerialDescriptor(m_port));
}
bool SerialDescriptor::operator==(const ControllerDescriptor& x) const{
    if (typeid(*this) != typeid(x)){
        return false;
    }
    return m_port.portName() == static_cast<const SerialDescriptor&>(x).m_port.portName();
}


const char* SerialDescriptor::type_name() const{
    return TYPENAME;
}
std::string SerialDescriptor::display_name() const{
    if (m_port.isNull()){
        return "";
    }
    return m_port.portName().toStdString() + " - " + m_port.description().toStdString();
}
void SerialDescriptor::load_json(const JsonValue& json){
    const std::string* name = json.to_string();
    if (name == nullptr || name->empty()){
        return;
    }
    m_port = (QSerialPortInfo(QString::fromStdString(*name)));
}
JsonValue SerialDescriptor::to_json() const{
    return m_port.isNull() ? "" : m_port.portName().toStdString();
}

std::unique_ptr<ControllerConnection> SerialDescriptor::open(
    Logger& logger,
    const ControllerRequirements& requirements
) const{
    return std::unique_ptr<ControllerConnection>(new SerialConnection(logger, *this, requirements));
}



std::vector<std::unique_ptr<const ControllerDescriptor>> get_all_devices(){
    std::vector<std::unique_ptr<const ControllerDescriptor>> ret;
    for (QSerialPortInfo& port : QSerialPortInfo::availablePorts()){
#ifdef _WIN32
        //  COM1 is never the correct port on Windows.
        if (port.portName() == "COM1"){
            continue;
        }
#endif
        ret.emplace_back(new SerialDescriptor(port));
    }
    return ret;
}





}
}
