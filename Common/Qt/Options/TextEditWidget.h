/*  Text Edit Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_TextEditWidget_H
#define PokemonAutomation_Options_TextEditWidget_H

#include <QWidget>
#include "Common/Cpp/Options/TextEditOption.h"
#include "ConfigWidget.h"

class QTextEdit;

namespace PokemonAutomation{



class TextEditWidget : public QWidget, public ConfigWidget{
public:
    ~TextEditWidget();
    TextEditWidget(QWidget& parent, TextEditOption& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;

private:
    class Box;

    TextEditOption& m_value;
    QTextEdit* m_box;
};



}
#endif
