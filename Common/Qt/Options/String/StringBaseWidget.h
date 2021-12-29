/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_StringBaseWidget_H
#define PokemonAutomation_StringBaseWidget_H

#include <QWidget>
#include <QLineEdit>
#include "StringBaseOption.h"

namespace PokemonAutomation{


class StringBaseWidget : public QWidget{
public:
    StringBaseWidget(QWidget& parent, StringBaseOption& value);
    void restore_defaults();

private:
    StringBaseOption& m_value;
    QLineEdit * m_line_edit;
};



}
#endif
