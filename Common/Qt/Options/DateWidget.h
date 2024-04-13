/*  Date Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_DateWidget_H
#define PokemonAutomation_Options_DateWidget_H

#include <QWidget>
#include "Common/Cpp/Options/DateOption.h"
#include "ConfigWidget.h"

class QDateEdit;
class QDateTimeEdit;

namespace PokemonAutomation{



#if 0
class DateWidget : public QWidget, public ConfigWidget{
public:
    ~DateWidget();
    DateWidget(QWidget& parent, DateOption& value);

    virtual void update_value() override;
    virtual void value_changed() override;

private:
    DateOption& m_value;
    QDateEdit* m_date_edit;
};
#endif


class DateTimeWidget : public QWidget, public ConfigWidget{
public:
    ~DateTimeWidget();
    DateTimeWidget(QWidget& parent, DateTimeOption& value);

    virtual void update_value() override;
    virtual void value_changed() override;

private:
    DateTimeOption& m_value;
    QDateTimeEdit* m_date_edit;
};


}
#endif
