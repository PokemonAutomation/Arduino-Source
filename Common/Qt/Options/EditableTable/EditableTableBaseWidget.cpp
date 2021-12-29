/*  Editable Table Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QHeaderView>
#include <QScrollBar>
#include <QLabel>
#include <QPushButton>
#include "EditableTableBaseWidget.h"

namespace PokemonAutomation{


EditableTableBaseWidget::EditableTableBaseWidget(QWidget& parent, EditableTableBaseOption& value)
    : QWidget(&parent)
    , m_value(value)
    , m_table(nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    if (!value.m_margin){
        layout->setContentsMargins(0, 0, 0, 0);
    }

    layout->addWidget(new QLabel(value.m_label, this));

    m_table = new AutoHeightTableWidget(this);
    layout->addWidget(m_table, 0, Qt::AlignTop);

    QStringList header = m_value.m_factory.make_header();
    header << "" << "";
    m_table->setColumnCount(header.size());
    m_table->setHorizontalHeaderLabels(header);

    QFont font;
    font.setBold(true);
    m_table->horizontalHeader()->setFont(font);
//    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    redraw_table();
}
void EditableTableBaseWidget::redraw_table(){
    m_table->setRowCount(0);

    for (const std::unique_ptr<EditableTableRow>& item : m_value.m_current){
        int row = m_table->rowCount();
        item->m_index = row;

        m_table->insertRow(row);

        std::vector<QWidget*> widgets = item->make_widgets(*m_table);
        int c = 0;
        int stop = (int)widgets.size();
        for (; c < stop; c++){
            m_table->setCellWidget(row, c, widgets[c]);
        }
        m_table->setCellWidget(row, c++, make_insert_button(*item));
        m_table->setCellWidget(row, c++, make_delete_button(*item));
    }

    //  Add row button.
    {
        int row = m_table->rowCount();
        m_table->insertRow(row);

        QPushButton* button = new QPushButton(m_table);
        button->setText("Add Row");
        m_table->setCellWidget(row, 0, button);
        connect(
            button, &QPushButton::clicked,
            this, [=](bool){
                int index = (int)m_value.size();
                insert_row(index, m_value.m_factory.make_row());
                if (index == 0){
                    m_table->resizeColumnsToContents();
                }
            }
        );
    }

    m_table->resizeColumnsToContents();
}
void EditableTableBaseWidget::insert_row(int index, std::unique_ptr<EditableTableRow> row){
    //  Insert the row.
    EditableTableRow& item = *row;
    m_value.add_row(index, std::move(row));
    m_table->insertRow(index);

    //  Populate widgets.
    {
        std::vector<QWidget*> widgets = item.make_widgets(*m_table);
        int c = 0;
        int stop = (int)widgets.size();
        for (; c < stop; c++){
            m_table->setCellWidget(index, c, widgets[c]);
        }
        m_table->setCellWidget(index, c++, make_insert_button(item));
        m_table->setCellWidget(index, c++, make_delete_button(item));
    }

    //  Update indices on all rows below.
    for (size_t c = index; c < m_value.size(); c++){
        m_value.m_current[c]->m_index = c;
    }

//    m_table->resizeColumnsToContents();
}
void EditableTableBaseWidget::delete_row(int index){
    m_value.remove_row(index);
    m_table->removeRow(index);

    //  Update indices on all rows below.
    for (size_t c = index; c < m_value.size(); c++){
        m_value.m_current[c]->m_index = c;
    }
}
QWidget* EditableTableBaseWidget::make_insert_button(EditableTableRow& row){
    QPushButton* button = new QPushButton(m_table);
    QFont font;
    font.setBold(true);
    button->setFont(font);
    button->setText("Insert");
    button->setMaximumWidth(60);

    connect(
        button, &QPushButton::clicked,
        this, [&](bool){
            insert_row((int)row.m_index, m_value.m_factory.make_row());
        }
    );

    return button;
}
QWidget* EditableTableBaseWidget::make_delete_button(EditableTableRow& row){
    QPushButton* button = new QPushButton(m_table);
    QFont font;
    font.setBold(true);
    button->setFont(font);
    button->setText("Delete");
    button->setMaximumWidth(60);

    connect(
        button, &QPushButton::clicked,
        this, [&](bool){
            delete_row((int)row.m_index);
        }
    );

    return button;
}
void EditableTableBaseWidget::restore_defaults(){
    m_value.restore_defaults();
    redraw_table();
}
void EditableTableBaseWidget::update_column_sizes(){
    m_table->resizeColumnsToContents();
}


}
