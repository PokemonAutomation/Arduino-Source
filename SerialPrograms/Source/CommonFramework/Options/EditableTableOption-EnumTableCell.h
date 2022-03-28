/*  Enum Table Cell
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Generate a dropdown menu in EditableTableOption for user to choose. The choice is
 *  bind to the input enum `value`.
 *  The reason to put this function in a separate header outside EditableTableOption.h
 *  is because this function must be a template defined in header and to avoid all the
 *  files that include EditableTableOption.h to have this template parsed by compiler.
 */

#ifndef PokemonAutomation_EditableTableOptionEnumTableCell_H
#define PokemonAutomation_EditableTableOptionEnumTableCell_H

#include <QString>
#include "Common/Qt/NoWheelComboBox.h"

namespace PokemonAutomation{


// Generate a dropdown menu for user to choose. The choice is bind to the input enum `value`.
// Pass the member var of EditableTableRow as `value` so that user editing this UI widget in a table
// row changes the value of the EditableTableRow automatically.
// T must be an enum that is based on int starting at 0.
template<typename T> QWidget* make_enum_table_cell(QWidget& parent, size_t num_enum_names, const QString* enum_names, T& value){
    QComboBox* box = new NoWheelComboBox(&parent);
    for(size_t i = 0; i < num_enum_names; i++){
        box->addItem(enum_names[i]);
    }
    box->setCurrentIndex((int)value);
    box->connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        box, [&value](int index){
            if (index < 0){
                index = 0;
            }
            value = (T)index;
        }
    );
    return box;
}







}
#endif
