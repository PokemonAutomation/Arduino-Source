/*  Editable Table Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_EditableTableWidget_H
#define PokemonAutomation_Options_EditableTableWidget_H

#include <QWidget>
#include "Common/Cpp/Options/EditableTableOption2.h"
#include "ConfigWidget.h"

namespace PokemonAutomation{

class AutoHeightTableWidget;


class EditableTableWidget2 : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~EditableTableWidget2();
    EditableTableWidget2(QWidget& parent, EditableTableOptionCore& value);

    virtual void update() override;
    virtual void value_changed() override;

    void update_column_sizes();

private:
    QWidget* make_clone_button(EditableTableRow2& row);
    QWidget* make_insert_button(EditableTableRow2& row);
    QWidget* make_delete_button(EditableTableRow2& row);

private:
    EditableTableOptionCore& m_value;
    AutoHeightTableWidget* m_table;
    std::vector<std::shared_ptr<EditableTableRow2>> m_current;
};




}
#endif
