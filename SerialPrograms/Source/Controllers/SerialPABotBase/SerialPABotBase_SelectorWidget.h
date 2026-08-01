/*  SerialPABotBase Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_SerialPABotBase_SelectorWidget_H
#define PokemonAutomation_Controllers_SerialPABotBase_SelectorWidget_H

#include <QFileInfo>
#include <QSerialPortInfo>
#include "Common/Qt/UiStateQtWidget.h"
#include "Common/Qt/NoWheelComboBox.h"
#include "Controllers/ControllerDescriptor.h"
#include "Controllers/ControllerSelectorWidget.h"
#include "SerialPABotBase_Descriptor.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace SerialPABotBase{



bool filter_serial_port(const QSerialPortInfo& port);




class SerialPABotBase_SelectorWidget
    : public NoWheelCompactComboBox
    , public UiComponentQtWidget
{
public:
    using ParentState = const SerialPABotBase_Descriptor;

    SerialPABotBase_SelectorWidget(
        QWidget& parent,
        const ParentState& current
    )
        : SerialPABotBase_SelectorWidget(static_cast<ControllerSelectorWidget&>(parent), &current)
    {}
    SerialPABotBase_SelectorWidget(
        ControllerSelectorWidget& parent,
        const ControllerDescriptor* current
    );

    void refresh_devices();

    virtual QWidget& widget() override{
        return *this;
    }


private:
    ControllerSelectorWidget& m_parent;
    std::vector<std::shared_ptr<ControllerDescriptor>> m_ports;
};





}
}
#endif
