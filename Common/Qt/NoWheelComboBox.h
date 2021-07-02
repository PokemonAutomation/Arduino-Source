/*  ComboBox without mouse wheel scrolling.
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NoWheelComboBox_H
#define PokemonAutomation_NoWheelComboBox_H

#include <QComboBox>

namespace PokemonAutomation{


class NoWheelComboBox : public QComboBox{
public:
    using QComboBox::QComboBox;
    virtual void wheelEvent(QWheelEvent* e) override{}
};



}
#endif
