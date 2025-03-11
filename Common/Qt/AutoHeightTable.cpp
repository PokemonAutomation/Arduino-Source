/*  QTableWidth with auto-adjusting height.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QHeaderView>
#include <QScrollBar>
#include "AutoHeightTable.h"

namespace PokemonAutomation{


void AutoHeightTableWidget::update_height(){
    int rows = this->rowCount();
    int total_height = 5;
    total_height += this->horizontalHeader()->height();
    total_height += this->horizontalScrollBar()->height();
    for (int c = 0; c < rows; c++){
        total_height += this->verticalHeader()->sectionSize(c);
    }
//    cout << total_height << endl;
    this->setMinimumHeight(total_height);
    this->setMaximumHeight(total_height);
//    this->adjustSize();
//    this->parentWidget()->adjustSize();
//    this->parentWidget()->parentWidget()->adjustSize();
}

void AutoHeightTableWidget::setRowCount(int rows){
    QTableWidget::setRowCount(rows);
    update_height();
}
void AutoHeightTableWidget::insertRow(int row){
    QTableWidget::insertRow(row);
    update_height();
}
void AutoHeightTableWidget::removeRow(int row){
    QTableWidget::removeRow(row);
    update_height();
}




}
