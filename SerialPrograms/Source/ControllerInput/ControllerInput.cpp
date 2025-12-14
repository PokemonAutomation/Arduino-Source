/*  Controller Input
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <vector>
#include "ControllerInput.h"

#include "Keyboard/GlobalKeyboardHidTracker.h"

namespace PokemonAutomation{



class GlobalInputSources{
public:
    GlobalInputSources(){
        //
        //  When we add more input types, add them here.
        //

        m_input_sources.emplace_back(&global_keyboard_tracker());

    }

    void stop(){
        for (ControllerInputSource* source : m_input_sources){
            source->stop();
        }
    }
    void clear_state(){
        for (ControllerInputSource* source : m_input_sources){
            source->clear_state();
        }
    }
    void add_listener(ControllerInputListener& listener){
        for (ControllerInputSource* source : m_input_sources){
            source->add_listener(listener);
        }
    }
    void remove_listener(ControllerInputListener& listener){
        for (ControllerInputSource* source : m_input_sources){
            source->remove_listener(listener);
        }
    }


private:
    std::vector<ControllerInputSource*> m_input_sources;
};






GlobalInputSources& global_input_sources(){
    static GlobalInputSources sources;
    return sources;
}




void global_input_stop(){
    global_input_sources().stop();
}
void global_input_clear_state(){
    global_input_sources().clear_state();
}
void global_input_add_listener(ControllerInputListener& listener){
    global_input_sources().add_listener(listener);
}
void global_input_remove_listener(ControllerInputListener& listener){
    global_input_sources().remove_listener(listener);
}




}
