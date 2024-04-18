/*  Date Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_DateWidget_H
#define PokemonAutomation_Options_DateWidget_H

#include <QWidget>
#include <QDateTimeEdit>
#include "Common/Cpp/Options/DateOption.h"
#include "ConfigWidget.h"

namespace PokemonAutomation{



class DateTimeCellWidget : public QDateTimeEdit, public ConfigWidget{
public:
    ~DateTimeCellWidget();
    DateTimeCellWidget(QWidget& parent, DateTimeCell& value);

    virtual void update_value() override;
    virtual void value_changed() override;

private:
    DateTimeCell& m_value;
};


class DateTimeOptionWidget : public QWidget, public ConfigWidget{
public:
    DateTimeOptionWidget(QWidget& parent, DateTimeOption& value);

private:
    DateTimeCellWidget* m_date_edit;
};


}
#endif
