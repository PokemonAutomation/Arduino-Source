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


#if 0
class NullControllerWidget : public QWidget, public UiComponentQtWidget{
public:
    using ParentState = const NullControllerDescriptor;

    NullControllerWidget(QWidget& parent, const ParentState& descriptor)
        : QWidget(&parent)
    {}
    virtual QWidget& widget() override{
        return *this;
    }
};
#endif


}
#endif
