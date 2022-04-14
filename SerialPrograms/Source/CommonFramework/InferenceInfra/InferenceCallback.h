/*  Inference Callback
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_InferenceCallback_H
#define PokemonAutomation_CommonFramework_InferenceCallback_H

#include <string>
#include <chrono>

namespace PokemonAutomation{

enum class InferenceType{
    VISUAL,
    AUDIO,
};


class InferenceCallback{
    //  Disable these to prevent accidental copying/slicing.
    InferenceCallback(const InferenceCallback&) = delete;
    void operator=(const InferenceCallback&) = delete;

public:
    virtual ~InferenceCallback() = default;

    InferenceType type() const{ return m_type; }
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




struct PeriodicInferenceCallback{
    InferenceCallback* callback;
    std::chrono::milliseconds period = std::chrono::milliseconds(0);    //  0 means user default

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
