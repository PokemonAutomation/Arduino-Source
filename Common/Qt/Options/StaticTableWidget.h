/*  Static Table Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_StaticTableWidget_H
#define PokemonAutomation_Options_StaticTableWidget_H

#include <QWidget>
#include "Common/Cpp/Options/StaticTableOption.h"
#include "ConfigWidget.h"

namespace PokemonAutomation{

class AutoHeightTableWidget;


class StaticTableWidget : public QWidget, public ConfigWidget{
public:
    ~StaticTableWidget();
    StaticTableWidget(QWidget& parent, StaticTableOption& value);

private:
    StaticTableOption& m_value;
    AutoHeightTableWidget* m_table;
    std::vector<std::shared_ptr<StaticTableRow>> m_current;
};




}
#endif
