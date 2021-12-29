/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EncounterFilterWidget_H
#define PokemonAutomation_PokemonBDSP_EncounterFilterWidget_H

#include <QComboBox>
#include "PokemonBDSP_EncounterFilterOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class EncounterFilterWidget : public QWidget, public ConfigWidget{
public:
    EncounterFilterWidget(QWidget& parent, EncounterFilterOption& value);

    virtual void restore_defaults() override;

private:
    EncounterFilterOption& m_value;

    QComboBox* m_shininess = nullptr;
    ConfigWidget* m_table = nullptr;
};



}
}
}
#endif
