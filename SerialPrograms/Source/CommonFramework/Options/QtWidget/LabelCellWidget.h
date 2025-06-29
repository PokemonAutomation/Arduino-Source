/*  Label Cell
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_LabelCellWidget_H
#define PokemonAutomation_Options_LabelCellWidget_H

#include <QWidget>
#include "Common/Qt/Options/ConfigWidget.h"
#include "CommonFramework/Options/LabelCellOption.h"

class QLabel;

namespace PokemonAutomation{



class LabelCellWidget : public QWidget, public ConfigWidget{
public:
    ~LabelCellWidget();
    LabelCellWidget(QWidget& parent, LabelCellOption& value);

private:
//    LabelCellOption& m_value;
    QLabel* m_icon = nullptr;
    QLabel* m_text;
};




}
#endif
