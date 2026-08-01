/*  Ui State Qt Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_UiStateQtWidget_H
#define PokemonAutomation_UiStateQtWidget_H

#include "Common/Cpp/UiWrapper.h"

class QWidget;

namespace PokemonAutomation{


class UiComponentQtWidget : public UiComponent{
public:
    virtual QWidget& widget() = 0;
};


template <typename UiWidgetType>
class RegisterUiStateQtWidget{
    using UiStateType = typename UiWidgetType::ParentState;

public:
    RegisterUiStateQtWidget(){
        UiStateType::m_ui_factory = [](UiStateType& state, void* params){
            QWidget* parent = (QWidget*)params;
            return UiWrapper(parent == nullptr, new UiWidgetType(*parent, state));
        };
    }

    static RegisterUiStateQtWidget initializer;
};

template <typename UiWidgetType>
RegisterUiStateQtWidget<UiWidgetType> RegisterUiStateQtWidget<UiWidgetType>::initializer;



}
#endif
