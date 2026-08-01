/*  sys-botbase Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_SysbotBase_SelectorWidget_H
#define PokemonAutomation_Controllers_SysbotBase_SelectorWidget_H

#include <QLineEdit>
#include "Common/Qt/UiStateQtWidget.h"
#include "Controllers/ControllerDescriptor.h"
#include "Controllers/ControllerSelectorWidget.h"
#include "SysbotBase_Descriptor.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace SysbotBase{



class TcpSysbotBase_SelectorWidget
    : public QLineEdit
    , public UiComponentQtWidget
{
public:
    using ParentState = const TcpSysbotBase_Descriptor;

    TcpSysbotBase_SelectorWidget(
        QWidget& parent,
        const ParentState& current
    )
        : TcpSysbotBase_SelectorWidget(static_cast<ControllerSelectorWidget&>(parent), &current)
    {}
    TcpSysbotBase_SelectorWidget(
        ControllerSelectorWidget& parent,
        const ControllerDescriptor* current
    );

    virtual QWidget& widget() override{
        return *this;
    }
};



}
}
#endif
