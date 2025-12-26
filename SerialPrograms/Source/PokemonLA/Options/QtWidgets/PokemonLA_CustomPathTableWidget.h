/*  Custom Path Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_CustomPathTableTableWidget_H
#define PokemonAutomation_PokemonLA_CustomPathTableTableWidget_H

#include <QWidget>
#include "Common/Qt/Options/ConfigWidget.h"
#include "PokemonLA/Options/PokemonLA_CustomPathTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



class CustomPathTableWidget : public QWidget, public ConfigWidget{
public:
    using ParentOption = CustomPathTableFromJubilife;

public:
    CustomPathTableWidget(QWidget& parent, CustomPathTableFromJubilife& value);

private:
    ConfigWidget* m_travel_location = nullptr;
    ConfigWidget* m_table_widget = nullptr;
};



}
}
}
#endif
