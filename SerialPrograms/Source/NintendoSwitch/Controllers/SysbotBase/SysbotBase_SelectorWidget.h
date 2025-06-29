/*  sys-botbase Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_SysbotBase_SelectorWidget_H
#define PokemonAutomation_Controllers_SysbotBase_SelectorWidget_H

#include <QLineEdit>
#include "Controllers/ControllerDescriptor.h"
#include "Controllers/ControllerSelectorWidget.h"
#include "SysbotBase_Descriptor.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace SysbotBase{



class TcpSysbotBase_SelectorWidget : public QLineEdit{
public:
    TcpSysbotBase_SelectorWidget(
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
            std::shared_ptr<const ControllerDescriptor> descriptor =
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
                std::shared_ptr<const ControllerDescriptor> selected(new TcpSysbotBase_Descriptor(
                    this->text().toStdString()
                ));

                std::shared_ptr<const ControllerDescriptor> current = parent.session().descriptor();
                if (*current == *selected){
                    return;
                }

                parent.session().set_device(std::move(selected));
            }
        );
    }
};



}
}
#endif
