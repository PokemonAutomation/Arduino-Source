/*  Editable Table Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_EditableTableWidget_H
#define PokemonAutomation_EditableTableWidget_H

#include "Common/Qt/Options/ConfigWidget.h"
#include "Common/Qt/Options/EditableTable/EditableTableBaseWidget.h"
#include "EditableTableOption.h"

namespace PokemonAutomation{


class EditableTableWidget : public EditableTableBaseWidget, public ConfigWidget{
public:
    EditableTableWidget(QWidget& parent, EditableTableOption& value)
        : EditableTableBaseWidget(parent, value)
        , ConfigWidget(value, *this)
    {}
    virtual void update() override{
        ConfigWidget::update();
        EditableTableBaseWidget::update_ui();
    }
};



}
#endif
