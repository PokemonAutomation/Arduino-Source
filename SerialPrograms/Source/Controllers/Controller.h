/*  Controller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Controllers_Controller_H
#define PokemonAutomation_Controllers_Controller_H

#include <string>
#include "Common/Compiler.h"
#include "Common/Cpp/Time.h"

class QKeyEvent;

namespace PokemonAutomation{


enum class ControllerType;


class AbstractController{
public:
    virtual ~AbstractController() = default;


public:
    //  Static Information

    virtual ControllerType controller_type() const = 0;

    //  If the controller is polled at a fixed interval, this is that interval.
    //  Otherwise, returns zero.
    virtual Milliseconds ticksize() const = 0;

    //  Some controllers are imprecise. This returns the variation.
    //  Zero means "tick precise".
    virtual Milliseconds timing_variation() const = 0;


public:
    //  Status

    virtual bool is_ready() const = 0;
    virtual std::string error_string() const = 0;


public:
    //  Keyboard Controls

    virtual void keyboard_release_all(){}
    virtual void keyboard_press(const QKeyEvent& event){}
    virtual void keyboard_release(const QKeyEvent& event){}
};




inline Milliseconds round_up_to_ticksize(Milliseconds ticksize, Milliseconds duration){
    if (ticksize == Milliseconds::zero()){
        return duration;
    }
    return (duration + ticksize - Milliseconds(1)) / ticksize * ticksize;
}



}
#endif
