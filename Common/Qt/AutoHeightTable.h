/*  QTableWidth with auto-adjusting height.
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AutoHeightTable_H
#define PokemonAutomation_AutoHeightTable_H

#include <QTableWidget>

namespace PokemonAutomation{


class AutoHeightTableWidget : public QTableWidget{
public:
    using QTableWidget::QTableWidget;

    void setRowCount(int rows);
    void insertRow(int row);
    void removeRow(int row);

    void update_height();
};


}
#endif
