/*  Editable Table Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QHeaderView>
#include <QScrollBar>
#include <QLabel>
#include <QPushButton>
#include "Common/Qt/AutoHeightTable.h"
#include "EditableTableWidget2.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


ConfigWidget* EditableTableOption2::make_ui(QWidget& parent){
    return new EditableTableWidget2(parent, *this);
}



EditableTableWidget2::~EditableTableWidget2(){
    m_value.remove_listener(*this);
    delete m_table;
}
EditableTableWidget2::EditableTableWidget2(QWidget& parent, EditableTableOption2& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
    , m_table(nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel* label = new QLabel(QString::fromStdString(value.label()), this);
    label->setWordWrap(true);
    layout->addWidget(label);

    m_table = new AutoHeightTableWidget(this);
    layout->addWidget(m_table, 0, Qt::AlignTop);

    QStringList header;
    for (const std::string& name : m_value.make_header()){
        header << QString::fromStdString(name);
    }
    header << "" << "" << "";
    m_table->setColumnCount(header.size());
    m_table->setHorizontalHeaderLabels(header);

    QFont font;
    font.setBold(true);
    m_table->horizontalHeader()->setFont(font);
//    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

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
                int index = (int)m_current.size();
                m_value.insert_row(index, m_value.make_row());
            }
        );
    }

    EditableTableWidget2::update();
    m_table->update_height();
//    EditableTableWidget2::value_changed();

//    m_table->resizeColumnsToContents();
//    m_table->resizeRowsToContents();

    value.add_listener(*this);
}

void EditableTableWidget2::update(){
    ConfigWidget::update();
    std::vector<std::shared_ptr<EditableTableRow2>> latest = m_value.current_refs();
//    cout << "latest.size() = " << latest.size() << endl;

    //  Iterate the old and new rows and resolve only the differences.
    size_t index_old = 0;
    size_t index_new = 0;
    while (true){
        //  End of both lists.
        if (index_old == m_current.size() && index_new == latest.size()){
            break;
        }

        //  Rows are the same. No change needed.
        if (index_old < m_current.size() && index_new < latest.size() &&
            m_current[index_old]->seqnum() == latest[index_new]->seqnum()
        ){
            index_old++;
            index_new++;
            continue;
        }

//        cout << index_old << " - " << index_new << endl;

        //  End of new list or row no longer valid. Remove it.
        if (index_new == latest.size() ||
            (index_old < m_current.size() && m_current[index_old]->index() == (size_t)0 - 1)
        ){
            //  QTableWidget::removeRow() does not immediately delete the
            //  widgets in that row. So we need to do it manually to drop the
            //  references to the row objects.
            int stop = m_table->columnCount();
            for (int c = 0; c < stop; c++){
                delete m_table->cellWidget((int)index_new, c);
            }
            m_table->removeRow((int)index_new);
            index_old++;
            continue;
        }

        //  Add the row from the new list.
        m_table->insertRow((int)index_new);

        //  Populate widgets.
        {
            EditableTableRow2& row = *latest[index_new];
            std::vector<ConfigOption*> cells = row.make_cells();
            int c = 0;
            int stop = (int)cells.size();
            for (; c < stop; c++){
                m_table->setCellWidget((int)index_new, c, &cells[c]->make_ui(*m_table)->widget());
            }
            m_table->setCellWidget((int)index_new, c++, make_clone_button(row));
            m_table->setCellWidget((int)index_new, c++, make_insert_button(row));
            m_table->setCellWidget((int)index_new, c++, make_delete_button(row));
        }

        index_new++;
    }

#if 0
    for (int r = 0; r < m_table->rowCount() - 1; r++){
        for (int c = 0; c < m_table->columnCount(); c++){
            cout << m_table->cellWidget(r, c)->sizeHint().width() << "  ";
        }
        cout << endl;
    }
#endif

//    cout << "latest.size() = " << latest.size() << endl;
    m_current = std::move(latest);
    m_table->resizeColumnsToContents();
    m_table->resizeRowsToContents();
}
void EditableTableWidget2::value_changed(){
    QMetaObject::invokeMethod(m_table, [=]{
        update();
    }, Qt::QueuedConnection);
}
void EditableTableWidget2::update_sizes(){
    m_table->resizeColumnsToContents();
    m_table->resizeRowsToContents();
}


QWidget* EditableTableWidget2::make_clone_button(EditableTableRow2& row){
    QPushButton* button = new QPushButton(m_table);

    QFont font;
    font.setBold(true);
    button->setFont(font);
    button->setText("Copy");
    button->setMaximumWidth(60);

    connect(
        button, &QPushButton::clicked,
        this, [&](bool){
            m_value.clone_row(row);
        }
    );

    return button;
}
QWidget* EditableTableWidget2::make_insert_button(EditableTableRow2& row){
    QPushButton* button = new QPushButton(m_table);

    QFont font;
    font.setBold(true);
    button->setFont(font);
    button->setText("Insert");
    button->setMaximumWidth(60);

    connect(
        button, &QPushButton::clicked,
        this, [&](bool){
            m_value.insert_row(row.index(), m_value.make_row());
        }
    );

    return button;
}
QWidget* EditableTableWidget2::make_delete_button(EditableTableRow2& row){
    QPushButton* button = new QPushButton(m_table);

    QFont font;
    font.setBold(true);
    button->setFont(font);
    button->setText("Delete");
    button->setMaximumWidth(60);

    connect(
        button, &QPushButton::clicked,
        this, [&](bool){
            m_value.remove_row(row);
        }
    );

    return button;
}




}
