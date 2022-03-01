/*  Inference Callback
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_InferenceCallback_H
#define PokemonAutomation_CommonFramework_InferenceCallback_H

#include <string>

namespace PokemonAutomation{

enum class InferenceType{
    VISUAL,
    AUDIO,
};


class InferenceCallback{
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



}
#endif
