/*  Panel Lists
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PanelLists_H
#define PokemonAutomation_PanelLists_H

#include <QTabWidget>

namespace PokemonAutomation{
namespace HexGenerator{


class MainWindow;


class PanelLists : public QTabWidget{
public:
    PanelLists(QWidget& parent, MainWindow& window);

private:
};


}
}
#endif
