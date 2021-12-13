/*  Editable Table Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonValue>
#include <QJsonArray>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QScrollBar>
#include <QLabel>
#include <QPushButton>
#include "EditableTableOptionBase.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


EditableTableBase::EditableTableBase(QString label, const EditableTableFactory& factory, bool margin)
    : m_label(std::move(label))
    , m_factory(factory)
    , m_margin(margin)
{}

std::vector<std::unique_ptr<EditableTableRow>> EditableTableBase::load_json(const QJsonValue& json){
    std::vector<std::unique_ptr<EditableTableRow>> table;
    for (const auto& row : json.toArray()){
        table.emplace_back(m_factory.make_row());
        table.back()->load_json(row);
    }
    return table;
}
QJsonValue EditableTableBase::to_json(const std::vector<std::unique_ptr<EditableTableRow>>& table) const{
    QJsonArray array;
    for (const std::unique_ptr<EditableTableRow>& row : table){
        array.append(row->to_json());
    }
    return array;
}
void EditableTableBase::load_default(const QJsonValue& json){
    m_default = load_json(json);
}
void EditableTableBase::load_current(const QJsonValue& json){
    m_current = load_json(json);
}
QJsonValue EditableTableBase::write_default() const{
    return to_json(m_default);
}
QJsonValue EditableTableBase::write_current() const{
    return to_json(m_current);
}

const EditableTableRow& EditableTableBase::operator[](size_t index) const{
    return *m_current[index];
}

QString EditableTableBase::check_validity() const{
    for (const std::unique_ptr<EditableTableRow>& item : m_current){
        QString error = item->check_validity();
        if (!error.isEmpty()){
            return error;
        }
    }
    return QString();
}
void EditableTableBase::restore_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> tmp;
    for (const std::unique_ptr<EditableTableRow>& item : m_default){
        tmp.emplace_back(item->clone());
    }
    m_current = std::move(tmp);
}
void EditableTableBase::add_row(size_t index, std::unique_ptr<EditableTableRow> row){
    m_current.insert(m_current.begin() + index, std::move(row));
}
void EditableTableBase::remove_row(size_t index){
    m_current.erase(m_current.begin() + index);
}



EditableTableBaseUI::EditableTableBaseUI(QWidget& parent, EditableTableBase& value)
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
void EditableTableBaseUI::redraw_table(){
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
void EditableTableBaseUI::insert_row(int index, std::unique_ptr<EditableTableRow> row){
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
void EditableTableBaseUI::delete_row(int index){
    m_value.remove_row(index);
    m_table->removeRow(index);

    //  Update indices on all rows below.
    for (size_t c = index; c < m_value.size(); c++){
        m_value.m_current[c]->m_index = c;
    }
}
QWidget* EditableTableBaseUI::make_insert_button(EditableTableRow& row){
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
QWidget* EditableTableBaseUI::make_delete_button(EditableTableRow& row){
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
void EditableTableBaseUI::restore_defaults(){
    m_value.restore_defaults();
    redraw_table();
}
void EditableTableBaseUI::update_column_sizes(){
    m_table->resizeColumnsToContents();
}






}
