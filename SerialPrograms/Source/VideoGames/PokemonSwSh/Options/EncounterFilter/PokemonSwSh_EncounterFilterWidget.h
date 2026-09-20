/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EncounterFilterWidget_H
#define PokemonAutomation_PokemonSwSh_EncounterFilterWidget_H

#include <QComboBox>
#include "Common/Qt/Options/ConfigWidget.h"
#include "PokemonSwSh_EncounterFilterOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{




#if 0
class EncounterFilterWidget : public QWidget, public ConfigWidget{
public:
    EncounterFilterWidget(QWidget& parent, EncounterFilterOption& value);

    virtual void update_value() override;
    virtual void update_visibility() override;

private:
    EncounterFilterOption& m_value;

    QComboBox* m_shininess = nullptr;
    ConfigWidget* m_table = nullptr;
};
#endif



}
}
}
#endif
