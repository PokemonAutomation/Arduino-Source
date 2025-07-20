/*  Color Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ColorWidget_H
#define PokemonAutomation_ColorWidget_H

#include <QWidget>
#include "Common/Cpp/Options/ColorOption.h"
#include "ConfigWidget.h"

class QLabel;
class QLineEdit;

namespace PokemonAutomation{



class ColorCellWidget : public QWidget, public ConfigWidget{
public:
    ~ColorCellWidget();
    ColorCellWidget(QWidget& parent, ColorCell& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;


private:
    ColorCell& m_value;
    QLineEdit* m_line_edit;
    QLabel* m_box;
};




}
#endif
