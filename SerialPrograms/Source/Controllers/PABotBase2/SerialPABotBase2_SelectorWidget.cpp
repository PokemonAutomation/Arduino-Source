/*  SerialPABotBase2 Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Controllers/SerialPABotBase/SerialPABotBase_SelectorWidget.h"
#include "SerialPABotBase2_Descriptor.h"
#include "SerialPABotBase2_SelectorWidget.h"

namespace PokemonAutomation{

template class RegisterUiStateQtWidget<SerialPABotBase::SerialPABotBase2_SelectorWidget>;

namespace SerialPABotBase{


SerialPABotBase2_SelectorWidget::SerialPABotBase2_SelectorWidget(
    ControllerSelectorWidget& parent,
    const ControllerDescriptor* current
)
    : NoWheelCompactComboBox(&parent)
    , m_parent(parent)
{
    SerialPortPoller::instance().begin_refresh_now();

//        cout << "SerialPABotBase(): " << current << endl;
    this->setMaxVisibleItems(32);

    if (current == nullptr || (
            current->interface_type != ControllerInterface::None &&
            current->interface_type != ControllerInterface::SerialPABotBase2
        )
    ){
        std::shared_ptr<ControllerDescriptor> descriptor =
            parent.session().option().get_descriptor_from_cache(ControllerInterface::SerialPABotBase2);
        if (!descriptor){
            descriptor.reset(new SerialPABotBase2_Descriptor());
        }
        parent.session().set_device(descriptor);
    }

    refresh_devices(SerialPortPoller::instance().ports());

    connect(
        this, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
        &parent, [this, &parent](int index){
            if (index < 0){
                return;
            }
            index = std::min(index, (int)m_ports.size() - 1);
            std::shared_ptr<ControllerDescriptor>& selected = m_ports[index];

            std::shared_ptr<ControllerDescriptor> current = parent.session().descriptor();
            if (*current == *selected){
                return;
            }

            parent.session().set_device(selected);
            refresh_devices(SerialPortPoller::instance().ports());
        }
    );

    SerialPortPoller::instance().add_listener(*this);
}

void SerialPABotBase2_SelectorWidget::refresh_devices(const QList<QSerialPortInfo>& ports){
//    SerialPortPoller::instance().begin_refresh_now();
//    cout << "Current = " << width() << " x " << height() << endl;
//    cout << "sizeHint = " << sizeHint().width() << " x " << sizeHint().height() << endl;
//    cout << "minimumContentsLength = " << this->minimumContentsLength() << endl;

    m_ports.clear();
    this->clear();

//    cout << "SerialPABotBase_SelectorWidget::refresh_devices()" << endl;


    m_ports.emplace_back(new SerialPABotBase2_Descriptor());
    for (const QSerialPortInfo& port : ports){
        if (filter_serial_port(port)){
            m_ports.emplace_back(
                new SerialPABotBase2_Descriptor(port.portName().toStdString())
            );
        }
    }

//    size_t width = 6;
    int index = 0;
    int c = 0;
    for (const auto& port : m_ports){
        QString display_name = QString::fromStdString(port->display_name());
//        width = std::max<size_t>(width, display_name.size());
        this->addItem(display_name);
        if (*m_parent.session().descriptor() == *m_ports[c]){
            index = c;
        }
        c++;
    }

    if (this->count() > this->maxVisibleItems()){
//        width++;
    }
//    setMinimumContentsLength((int)width);
    setCurrentIndex(index);
}




}
}
