/*  Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Controllers_Controller_H
#define PokemonAutomation_Controllers_Controller_H

#include "Common/Compiler.h"

class QKeyEvent;

namespace PokemonAutomation{


class Controller{
public:
    virtual void keyboard_release_all(){}
    virtual void keyboard_press(const QKeyEvent& event){}
    virtual void keyboard_release(const QKeyEvent& event){}
};




}
#endif
