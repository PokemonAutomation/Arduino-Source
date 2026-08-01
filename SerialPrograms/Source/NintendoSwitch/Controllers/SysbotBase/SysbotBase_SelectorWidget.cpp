/*  sys-botbase Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "SysbotBase_SelectorWidget.h"

namespace PokemonAutomation{

template class RegisterUiStateQtWidget<SysbotBase::TcpSysbotBase_SelectorWidget>;

namespace SysbotBase{



TcpSysbotBase_SelectorWidget::TcpSysbotBase_SelectorWidget(
    ControllerSelectorWidget& parent,
    const ControllerDescriptor* current
)
    : QLineEdit(&parent)
{
//        cout << "TcpSysbotBase()" << endl;

    QSizePolicy policy;
    policy.setHorizontalStretch(3);
    this->setSizePolicy(policy);

    this->setPlaceholderText("192.168.0.100:6000");

    if (current == nullptr || current->interface_type != ControllerInterface::TcpSysbotBase){
        std::shared_ptr<ControllerDescriptor> descriptor =
            parent.session().option().get_descriptor_from_cache(ControllerInterface::TcpSysbotBase);
        if (!descriptor){
            descriptor.reset(new TcpSysbotBase_Descriptor());
        }
        parent.session().set_device(descriptor);
    }
    this->setText(QString::fromStdString(parent.session().descriptor()->display_name()));

    connect(
        this, &QLineEdit::editingFinished,
        &parent, [this, &parent](){
            std::shared_ptr<ControllerDescriptor> selected(new TcpSysbotBase_Descriptor(
                this->text().toStdString()
            ));

            std::shared_ptr<ControllerDescriptor> current = parent.session().descriptor();
            if (*current == *selected){
                return;
            }

            parent.session().set_device(std::move(selected));
        }
    );
}





}
}
