/*  Box Float Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_BoxFloatWidget_H
#define PokemonAutomation_BoxFloatWidget_H

#include <QLineEdit>
#include "Common/Cpp/Options/BoxFloatOption.h"
#include "ConfigWidget.h"

namespace PokemonAutomation{


class BoxFloatWidget : public QWidget, public ConfigWidget{
public:
    ~BoxFloatWidget();
    BoxFloatWidget(QWidget& parent, BoxFloatOption& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;

private:
    BoxFloatOption& m_value;
    QLineEdit* m_x;
    QLineEdit* m_y;
    QLineEdit* m_width;
    QLineEdit* m_height;
    QLineEdit* m_array;
};





}
#endif
