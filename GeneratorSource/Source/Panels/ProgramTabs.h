/*  Program Tabs
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ProgramTabs_H
#define PokemonAutomation_ProgramTabs_H

#include <QTabWidget>

namespace PokemonAutomation{


class MainWindow;


class ProgramTabs : public QTabWidget{
public:
    ProgramTabs(QWidget& parent, MainWindow& window);

private:
};


}
#endif
