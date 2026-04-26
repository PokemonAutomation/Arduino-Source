/*  Rumble Listener
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_RumbleListener_H
#define PokemonAutomation_RumbleListener_H

#include "CommonTools/InferenceCallbacks/InferenceCallback.h"

namespace PokemonAutomation{


struct RumbleListener{
    virtual void on_rumble(double magnitude) = 0;
};


template <typename ControllerType>
class RumbleWatcher : public ExternalInferenceCallback, public RumbleListener{
public:
    RumbleWatcher(ControllerType& controller, double magnitude_threshold)
        : ExternalInferenceCallback("RumbleWatcher")
        , m_controller(controller)
        , m_magnitude_threshold(magnitude_threshold)
        , m_max_magnitude(0)
    {
        controller.add_listener(*this);
    }
    ~RumbleWatcher(){
        m_controller.remove_listener(*this);
    }

    double max_magnitude() const{
        return m_max_magnitude;
    }

    virtual void on_rumble(double magnitude) override{
        m_max_magnitude = std::max(m_max_magnitude, magnitude);
        if (magnitude > m_magnitude_threshold){
            cancel(nullptr);
        }
    }

private:
    ControllerType& m_controller;
    const double m_magnitude_threshold;
    double m_max_magnitude;
};




}
#endif
