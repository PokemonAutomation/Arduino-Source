/*  MAC Address option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_MacAddressWidget_H
#define PokemonAutomation_MacAddressWidget_H

#include <QLineEdit>
#include "Common/Cpp/Options/MacAddressOption.h"
#include "ConfigWidget.h"

namespace PokemonAutomation{




class MacAddressCellWidget : public QLineEdit, public ConfigWidget{
public:
    using ParentState = MacAddressCell;

public:
    ~MacAddressCellWidget();
    MacAddressCellWidget(QWidget& parent, MacAddressCell& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;

private:
    MacAddressCell& m_value;
};




}
#endif
