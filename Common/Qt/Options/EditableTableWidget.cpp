/*  Editable Table Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QFileDialog>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QScrollBar>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/AutoHeightTable.h"
#include "EditableTableWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


ConfigWidget* EditableTableOption::make_QtWidget(QWidget& parent){
    return new EditableTableWidget(parent, *this);
}



EditableTableWidget::~EditableTableWidget(){
    m_value.remove_listener(*this);
    delete m_table;
}
EditableTableWidget::EditableTableWidget(QWidget& parent, EditableTableOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
    , m_table(nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    if (!value.label().empty()){
        QLabel* label = new QLabel(QString::fromStdString(value.label()), this);
        label->setWordWrap(true);
        label->setTextFormat(Qt::RichText);
        label->setTextInteractionFlags(Qt::TextBrowserInteraction);
        label->setOpenExternalLinks(true);
        layout->addWidget(label);
    }

    m_table = new AutoHeightTableWidget(this);
    layout->addWidget(m_table, 0, Qt::AlignTop);
//    m_table->setMouseTracking(false);

    QStringList header;
    for (const std::string& name : m_value.make_header()){
        header << QString::fromStdString(name);
    }
    header << "" << "" << "";
    m_table->setColumnCount(int(header.size()));
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
            this, [this](bool){
                int index = (int)m_current.size();
                m_value.insert_row(index, m_value.make_row());
            }
        );
    }

    EditableTableWidget::update_value();
//    EditableTableWidget2::value_changed();

//    m_table->resizeColumnsToContents();
//    m_table->resizeRowsToContents();
    m_table->update_height();


    if (value.saveload_enabled()){
        QHBoxLayout* buttons = new QHBoxLayout();
        layout->addLayout(buttons);
        {
            QPushButton* button = new QPushButton("Load Table", this);
            buttons->addWidget(button, 1);
            connect(
                button, &QPushButton::clicked,
                this, [this, &value](bool){
                    std::string path = QFileDialog::getOpenFileName(
                        this,
                        tr("Select a file to load."), "", tr("JSON files (*.json)")
                    ).toStdString();
                    if (path.empty()){
                        return;
                    }
                    value.load_json(load_json_file(path));
                }
            );
        }
        {
            QPushButton* button = new QPushButton("Save Table", this);
            buttons->addWidget(button, 1);
            connect(
                button, &QPushButton::clicked,
                this, [this, &value](bool){
                    std::string path = QFileDialog::getSaveFileName(
                        this,
                        tr("Select a file name to save to."), "", tr("JSON files (*.json)")
                    ).toStdString();
                    if (path.empty()){
                        return;
                    }
                    JsonValue json = value.to_json();
                    json.dump(path);
                }
            );
        }
        {
            QPushButton* button = new QPushButton("Restore Defaults", this);
            buttons->addWidget(button, 1);
            connect(
                button, &QPushButton::clicked,
                this, [&value](bool){
                    QMessageBox::StandardButton button = QMessageBox::question(
                        nullptr,
                        "Restore Defaults",
                        "Are you sure you wish to this table back to defaults? This will wipe the current table.",
                        QMessageBox::Ok | QMessageBox::Cancel
                    );
                    if (button == QMessageBox::Ok){
                        value.restore_defaults();
                    }
                }
            );
        }
        buttons->addStretch(2);
    }

    value.add_listener(*this);
}

void EditableTableWidget::update_value(){
    std::vector<std::shared_ptr<EditableTableRow>> latest = m_value.current_refs();
//    cout << "latest.size() = " << latest.size() << endl;

//    //  Adding cells to a table overwrites their visibility. Store all the cells
//    //  here so we can correct their visibility later.
//    std::vector<ConfigWidget*> cell_widgets;

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
            EditableTableRow& row = *latest[index_new];
            std::vector<ConfigOption*> cells = row.make_cells();
            int c = 0;
            int stop = (int)cells.size();
            for (; c < stop; c++){
                //  Wrap the widget so that it preserves its visibility.
                //  QTableWidget for some reason forces the visibility of its
                //  cells to visible.
//                cout << "make cell widget" << endl;
                QWidget* widget = &cells[c]->make_QtWidget(*m_table)->widget();
                QWidget* cell_widget = new QWidget(this);
                QVBoxLayout* layout = new QVBoxLayout(cell_widget);
                layout->setContentsMargins(0, 0, 0, 0);
//                cell_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                layout->addWidget(widget);
//                cell_widgets.emplace_back(widget);
                m_table->setCellWidget((int)index_new, c, cell_widget);
//                cell_widget->update();
//                cout << "cell_widget->width() = " << cell_widget->width() << endl;
//                cout << "cell_widget->sizeHint().width() = " << cell_widget->sizeHint().width() << endl;
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
    EditableTableWidget::update_sizes();

#if 0
    for (ConfigWidget* cell : cell_widgets){
        cout << "Before: " << cell->widget().isVisible() << endl;
        cell->update_visibility();
        cout << "After: " << cell->widget().isVisible() << endl;
    }
#endif
}
void EditableTableWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(m_table, [this]{
        update_value();
    }, Qt::QueuedConnection);
}
void EditableTableWidget::update_sizes(){
//    cout << "update_sizes()" << endl;
    QMetaObject::invokeMethod(m_table, [this]{
//        cout << "before = " << m_table->cellWidget(0, 1)->width() << endl;
        m_table->resizeColumnsToContents();
        m_table->resizeRowsToContents();
        m_table->update_height();
//        cout << "after = " << m_table->cellWidget(0, 1)->width() << endl;
    }, Qt::QueuedConnection);
}



QWidget* EditableTableWidget::make_clone_button(EditableTableRow& row){
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
QWidget* EditableTableWidget::make_insert_button(EditableTableRow& row){
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
QWidget* EditableTableWidget::make_delete_button(EditableTableRow& row){
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
