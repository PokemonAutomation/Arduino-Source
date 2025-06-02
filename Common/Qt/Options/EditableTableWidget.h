/*  Editable Table Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_EditableTableWidget_H
#define PokemonAutomation_Options_EditableTableWidget_H

#include <QWidget>
#include "Common/Cpp/Options/EditableTableOption.h"
#include "ConfigWidget.h"

namespace PokemonAutomation{

class AutoHeightTableWidget;


class EditableTableWidget : public QWidget, public ConfigWidget{
public:
    ~EditableTableWidget();
    EditableTableWidget(QWidget& parent, EditableTableOption& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;

    void update_sizes();

private:
    QWidget* make_clone_button(EditableTableRow& row);
    QWidget* make_insert_button(EditableTableRow& row);
    QWidget* make_delete_button(EditableTableRow& row);

private:
    EditableTableOption& m_value;
    AutoHeightTableWidget* m_table;
    std::vector<std::shared_ptr<EditableTableRow>> m_current;
};




}
#endif
