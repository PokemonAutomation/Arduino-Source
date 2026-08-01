/*  SerialPABotBase Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Controllers/SerialPortPollerQt.h"
#include "SerialPABotBase_Descriptor.h"
#include "SerialPABotBase_SelectorWidget.h"

namespace PokemonAutomation{

template class RegisterUiStateQtWidget<SerialPABotBase::SerialPABotBase_SelectorWidget>;

namespace SerialPABotBase{



bool filter_serial_port(const QSerialPortInfo& port){
#ifdef _WIN32
    //  COM1 is never the correct port on Windows.
    if (port.portName() == "COM1"){
        return false;
    }
#endif

#if defined(__APPLE__)
    // exlude tty
    if (port.portName().startsWith("tty.")){
        return false;
    }
    // exclude system builtin serial ports
    if (port.portName() == "cu.debug-console" ||
        port.portName() == "cu.Bluetooth-Incoming-Port"
    ){
        return false;
    }
#endif

#if defined(__linux__)
    const QString path = port.systemLocation();

    QFileInfo file(path);
    // Exclude devices that don't have Read/Write access
    if (!(file.isReadable() && file.isWritable()))
    {
        return false;
    }
#endif

    return true;
}





SerialPABotBase_SelectorWidget::SerialPABotBase_SelectorWidget(
    ControllerSelectorWidget& parent,
    const ControllerDescriptor* current
)
    : NoWheelCompactComboBox(&parent)
    , m_parent(parent)
{
//        cout << "SerialPABotBase(): " << current << endl;
    this->setMaxVisibleItems(32);

    if (current == nullptr || (
            current->interface_type != ControllerInterface::None &&
            current->interface_type != ControllerInterface::SerialPABotBase
        )
    ){
        std::shared_ptr<ControllerDescriptor> descriptor =
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
            std::shared_ptr<ControllerDescriptor>& selected = m_ports[index];

            std::shared_ptr<ControllerDescriptor> current = parent.session().descriptor();
            if (*current == *selected){
                return;
            }

            parent.session().set_device(selected);
            refresh_devices();
        }
    );
}


void SerialPABotBase_SelectorWidget::refresh_devices(){
//    cout << "Current = " << width() << " x " << height() << endl;
//    cout << "sizeHint = " << sizeHint().width() << " x " << sizeHint().height() << endl;
//    cout << "minimumContentsLength = " << this->minimumContentsLength() << endl;

    m_ports.clear();
    this->clear();

//    cout << "SerialPABotBase_SelectorWidget::refresh_devices()" << endl;


    m_ports.emplace_back(new SerialPABotBase_Descriptor());
    for (QSerialPortInfo& port : SerialPortPoller::instance().ports()){
        if (filter_serial_port(port)){
            m_ports.emplace_back(
                new SerialPABotBase_Descriptor(port.portName().toStdString())
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
