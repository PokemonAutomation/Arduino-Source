/*  SerialPABotBase2 Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_SerialPABotBase2_SelectorWidget_H
#define PokemonAutomation_Controllers_SerialPABotBase2_SelectorWidget_H

#include <QSerialPortInfo>
#include "Common/Qt/UiStateQtWidget.h"
#include "Common/Qt/NoWheelComboBox.h"
#include "Controllers/ControllerDescriptor.h"
#include "Controllers/ControllerSelectorWidget.h"
#include "Controllers/SerialPortPollerQt.h"
#include "SerialPABotBase2_Descriptor.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace SerialPABotBase{




class SerialPABotBase2_SelectorWidget
    : public NoWheelCompactComboBox
    , public UiComponentQtWidget
    , public SerialPortPoller::Listener
{
public:
    using ParentState = const SerialPABotBase2_Descriptor;

    SerialPABotBase2_SelectorWidget(
        QWidget& parent,
        const ParentState& current
    )
        : SerialPABotBase2_SelectorWidget(static_cast<ControllerSelectorWidget&>(parent), &current)
    {}
    SerialPABotBase2_SelectorWidget(
        ControllerSelectorWidget& parent,
        const ControllerDescriptor* current
    );
    ~SerialPABotBase2_SelectorWidget(){
        SerialPortPoller::instance().remove_listener(*this);
    }

    void refresh_devices(const QList<QSerialPortInfo>& ports);

    virtual QWidget& widget() override{
        return *this;
    }
    virtual void on_serial_ports_changed(const QList<QSerialPortInfo>& ports) override{
        refresh_devices(ports);
    }


private:
    ControllerSelectorWidget& m_parent;
    std::vector<std::shared_ptr<ControllerDescriptor>> m_ports;
};





}
}
#endif
