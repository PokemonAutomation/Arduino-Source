/*  Static Table Widget
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
#include <QTableWidget>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/AutoHeightTable.h"
#include "StaticTableWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


ConfigWidget* StaticTableOption::make_QtWidget(QWidget& parent){
    return new StaticTableWidget(parent, *this);
}



StaticTableWidget::~StaticTableWidget(){}
StaticTableWidget::StaticTableWidget(QWidget& parent, StaticTableOption& value)
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
        layout->addWidget(label);
    }

    m_table = new AutoHeightTableWidget(this);
    layout->addWidget(m_table, 0, Qt::AlignTop);

    const std::vector<StaticTableRow*>& table = value.table();
//    cout << "table = " << table.size() << endl;

    QStringList header;
    for (const std::string& name : m_value.make_header()){
        header << QString::fromStdString(name);
    }
    m_table->setColumnCount(int(header.size()));
    m_table->setRowCount((int)table.size());
    m_table->setHorizontalHeaderLabels(header);

    QFont font;
    font.setBold(true);
    m_table->horizontalHeader()->setFont(font);
//    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    for (size_t r = 0; r < table.size(); r++){
        std::vector<ConfigOption*> cells = table[r]->make_cells();
        for (size_t c = 0; c < cells.size(); c++){
            m_table->setCellWidget((int)r, (int)c, &cells[c]->make_QtWidget(*this)->widget());
        }
    }

    m_table->resizeColumnsToContents();
    m_table->resizeRowsToContents();
    m_table->update_height();

    if (value.saveload_enabled()){
        QHBoxLayout* buttons = new QHBoxLayout();
        layout->addLayout(buttons);
        {
            QPushButton* load_button = new QPushButton("Load Table", this);
            buttons->addWidget(load_button, 1);
            connect(
                load_button, &QPushButton::clicked,
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
            QPushButton* save_button = new QPushButton("Save Table", this);
            buttons->addWidget(save_button, 1);
            connect(
                save_button, &QPushButton::clicked,
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
        buttons->addStretch(2);
    }
}



}
