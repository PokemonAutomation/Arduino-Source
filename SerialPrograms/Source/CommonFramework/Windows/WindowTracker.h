/*  WindowTracker.h
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_WindowTracker_H
#define PokemonAutomation_WindowTracker_H

class QMainWindow;

namespace PokemonAutomation{


void add_window(QMainWindow& window);
void remove_window(QMainWindow& window);
void close_all_windows();


}
#endif
