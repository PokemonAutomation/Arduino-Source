/*  SerialPABotBase Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_SerialPABotBase_SelectorWidget_H
#define PokemonAutomation_Controllers_SerialPABotBase_SelectorWidget_H

#include "Common/Qt/NoWheelComboBox.h"
#include "Controllers/ControllerDescriptor.h"
#include "Controllers/ControllerSelectorWidget.h"
#include "Controllers/NullController.h"
#include "SerialPABotBase_Descriptor.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace SerialPABotBase{




class SerialPABotBase_SelectorWidget : public NoWheelComboBox{
public:
    SerialPABotBase_SelectorWidget(
        ControllerSelectorWidget& parent,
        const ControllerDescriptor* current
    )
        : NoWheelComboBox(&parent)
        , m_parent(parent)
    {
//        cout << "SerialPABotBase(): " << current << endl;

        if (current == nullptr || (
                current->interface_type != ControllerInterface::None &&
                current->interface_type != ControllerInterface::SerialPABotBase
            )
        ){
            std::shared_ptr<const ControllerDescriptor> descriptor =
                parent.session().option().get_descriptor_from_cache(ControllerInterface::SerialPABotBase);
            if (!descriptor){
                descriptor.reset(new SerialPABotBase_Descriptor());
            }
            parent.session().set_device(descriptor);
        }

        refresh_devices();

        connect(
            this, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
            &parent, [this, &parent](int index){
                if (index < 0){
                    return;
                }
                index = std::min(index, (int)m_ports.size() - 1);
                std::shared_ptr<const ControllerDescriptor>& selected = m_ports[index];

                std::shared_ptr<const ControllerDescriptor> current = parent.session().descriptor();
                if (*current == *selected){
                    return;
                }

                parent.session().set_device(selected);
                refresh_devices();
            }
        );
    }

    void refresh_devices(){
        m_ports.clear();
        this->clear();

//        cout << "SerialPABotBase_SelectorWidget::refresh_devices()" << endl;

        m_ports.emplace_back(new NullControllerDescriptor());
        for (QSerialPortInfo& port : QSerialPortInfo::availablePorts()){
#ifdef _WIN32
            //  COM1 is never the correct port on Windows.
            if (port.portName() == "COM1"){
                continue;
            }
#endif
            m_ports.emplace_back(new SerialPABotBase_Descriptor(port));
        }

        int index = 0;
        int c = 0;
        for (const auto& port : m_ports){
            this->addItem(QString::fromStdString(port->display_name()));
            if (*m_parent.session().descriptor() == *m_ports[c]){
                index = c;
            }
            c++;
        }

        setCurrentIndex(index);
    }


private:
    ControllerSelectorWidget& m_parent;
    std::vector<std::shared_ptr<const ControllerDescriptor>> m_ports;
};





}
}
#endif
