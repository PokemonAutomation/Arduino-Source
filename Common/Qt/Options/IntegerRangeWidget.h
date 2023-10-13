/*  Integer Range Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_IntegerRangeWidget_H
#define PokemonAutomation_IntegerRangeWidget_H

#include <QLineEdit>
#include "ConfigWidget.h"
#include "Common/Cpp/Options/IntegerRangeOption.h"

namespace PokemonAutomation{



template <typename Type>
class IntegerRangeCellWidget : public QWidget, public ConfigWidget{
public:
    ~IntegerRangeCellWidget();
    IntegerRangeCellWidget(QWidget& parent, IntegerRangeCell<Type>& value);

    virtual void update_value() override;
    virtual void value_changed() override;

private:
    IntegerRangeCell<Type>& m_value;
    QLineEdit* m_lo;
    QLineEdit* m_hi;
};






}
#endif
