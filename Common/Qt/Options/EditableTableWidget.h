/*  Editable Table Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_EditableTableWidget_H
#define PokemonAutomation_Options_EditableTableWidget_H

#include <QWidget>
#include <QLabel>
#include "Common/Cpp/Options/EditableTableOption.h"
#include "ConfigWidget.h"

namespace PokemonAutomation{

class AutoHeightTableWidget;


class EditableTableWidget : public QWidget, public ConfigWidget{
public:
    using ParentOption = EditableTableOption;

public:
    ~EditableTableWidget();
    EditableTableWidget(QWidget& parent, EditableTableOption& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;

    void update_sizes();

private:
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

    QWidget* make_clone_button(EditableTableRow& row);
    QWidget* make_insert_button(EditableTableRow& row);
    QWidget* make_delete_button(EditableTableRow& row);

    class Label : public QLabel{
    public:
        using QLabel::QLabel;
        virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

        EditableTableWidget* m_parent;
    };

private:
    EditableTableOption& m_value;
    AutoHeightTableWidget* m_table;
    QWidget* m_expand_text;
    std::vector<std::shared_ptr<EditableTableRow>> m_current;
    bool m_expanded = true;
};




}
#endif
