/*  Floating-Point Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_FloatingPointBaseWidget_H
#define PokemonAutomation_FloatingPointBaseWidget_H

#include <QWidget>
#include <QLineEdit>
#include "FloatingPointBaseOption.h"

namespace PokemonAutomation{


class FloatingPointBaseWidget : public QWidget{
public:
    FloatingPointBaseWidget(QWidget& parent, FloatingPointBaseOption& value);

    void restore_defaults();
    void update_ui();

private:
    FloatingPointBaseOption& m_value;
    QLineEdit* m_box;
};



}
#endif
