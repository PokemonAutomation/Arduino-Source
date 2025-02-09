/*  Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Controllers_Controller_H
#define PokemonAutomation_Controllers_Controller_H

#include <string>
#include "Common/Compiler.h"

class QKeyEvent;

namespace PokemonAutomation{


class AbstractController{
public:
    virtual ~AbstractController() = default;

    virtual bool is_ready() const = 0;
    virtual std::string error_string() const = 0;

public:
    //  Keyboard Controls
    virtual void keyboard_release_all(){}
    virtual void keyboard_press(const QKeyEvent& event){}
    virtual void keyboard_release(const QKeyEvent& event){}
};




}
#endif
