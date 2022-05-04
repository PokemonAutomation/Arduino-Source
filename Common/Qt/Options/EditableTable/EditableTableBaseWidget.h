/*  Editable Table Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_EditableTableBaseWidget_H
#define PokemonAutomation_EditableTableBaseWidget_H

#include <QTableWidget>
#include "Common/Qt/AutoHeightTable.h"
#include "EditableTableBaseOption.h"

namespace PokemonAutomation{


class EditableTableBaseWidget : public QWidget{
public:
    EditableTableBaseWidget(QWidget& parent, EditableTableBaseOption& value);

    void restore_defaults();
    void update_ui();

    void update_column_sizes();

private:
    void insert_row(int index, std::unique_ptr<EditableTableRow> row);
    void delete_row(int index);
    QWidget* make_insert_button(EditableTableRow& row);
    QWidget* make_delete_button(EditableTableRow& row);

private:
    EditableTableBaseOption& m_value;
    AutoHeightTableWidget* m_table;
};



}
#endif
