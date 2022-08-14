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

namespace PokemonAutomation{



class DateWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~DateWidget();
    DateWidget(QWidget& parent, DateOption& value);
    virtual void restore_defaults() override;
    virtual void update_ui() override;
    virtual void value_changed() override;

private:
    DateOption& m_value;
    QDateEdit* m_date_edit;
};



}
#endif
