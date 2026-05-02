/*  Inference Callback
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_InferenceCallback_H
#define PokemonAutomation_CommonTools_InferenceCallback_H

#include <string>
#include <chrono>
#include "Common/Cpp/CancellableScope.h"

namespace PokemonAutomation{

class Cancellable;

enum class InferenceType{
    EXTERNAL,
    VISUAL,
    AUDIO,
};

//  Base class for an inference object to be called perioridically by
//  inference routines in InferenceRoutines.h.
class InferenceCallback{
    //  Disable these to prevent accidental copying/slicing.
    InferenceCallback(const InferenceCallback&) = delete;
    void operator=(const InferenceCallback&) = delete;

public:
    virtual ~InferenceCallback() = default;

    //  Is it a visual or audio inference.
    InferenceType type() const{ return m_type; }
    //  Name of the inference object.
    const std::string& label() const{ return m_label; }


protected:
    InferenceCallback(InferenceType type, std::string label)
        : m_type(type)
        , m_label(label)
    {}


private:
    InferenceType m_type;
    std::string m_label;
};


class ExternalInferenceCallback : public InferenceCallback, public Cancellable{
public:
    ExternalInferenceCallback(std::string label)
        : InferenceCallback(InferenceType::EXTERNAL, std::move(label))
    {}
};



//  Used by inference routines in InferenceRoutines.h.
//  The struct contains an inference callback and its inference period: how long
//  should an inference routine wait before calling the callback object again.
struct PeriodicInferenceCallback{
    InferenceCallback* callback = nullptr;

    //  Inference period. 0 value means the inference routine should use the
    //  default inference period, which is set as a parameter to the inference
    //  routine.
    std::chrono::milliseconds period = std::chrono::milliseconds(0);

    PeriodicInferenceCallback(){}
    PeriodicInferenceCallback(
        InferenceCallback& p_callback,
        std::chrono::milliseconds p_period = std::chrono::milliseconds(0)
    )
        : callback(&p_callback)
        , period(p_period)
    {}
};



}
#endif
