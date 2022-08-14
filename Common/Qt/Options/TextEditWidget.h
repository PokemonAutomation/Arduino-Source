/*  Text Edit Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_TextEditWidget_H
#define PokemonAutomation_Options_TextEditWidget_H

#include <QWidget>
#include "Common/Cpp/Options/TextEditOption.h"
#include "ConfigWidget.h"

class QTextEdit;

namespace PokemonAutomation{



class TextEditWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~TextEditWidget();
    TextEditWidget(QWidget& parent, TextEditOption& value);

    virtual void update_ui() override;
    virtual void restore_defaults() override;
    virtual void value_changed() override;

private:
    class Box;

    TextEditOption& m_value;
    QTextEdit* m_box;
};



}
#endif
