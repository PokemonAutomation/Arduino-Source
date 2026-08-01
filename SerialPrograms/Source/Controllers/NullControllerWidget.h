/*  Null Controller Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_NullControllerWidget_H
#define PokemonAutomation_Controllers_NullControllerWidget_H

#include <QWidget>
#include "Common/Qt/UiStateQtWidget.h"
#include "NullController.h"

namespace PokemonAutomation{




class NullControllerWidget : public QWidget, public UiComponentQtWidget{
public:
    using ParentOption = const ControllerDescriptor;

    NullControllerWidget(QWidget& parent, const ControllerDescriptor& descriptor)
        : QWidget(&parent)
    {}
    virtual QWidget& widget() override{
        return *this;
    }
};




}
#endif
