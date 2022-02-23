/*  Audio Inference Callback
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_AudioInferenceCallback_H
#define PokemonAutomation_CommonFramework_AudioInferenceCallback_H

#include <string>
#include <vector>
#include <memory>
#include "Common/Compiler.h"

class QImage;

namespace PokemonAutomation{

class AudioSpectrum;
class AudioFeed;


class AudioInferenceCallback{
public:
    AudioInferenceCallback(std::string label)
        : m_label(label)
    {}

    const std::string& label() const{ return m_label; }

    //  Process new spectrums and do inferences on them.
    //  Return true if the inference session should stop.
    //  Input spectrums are ordered from newest (largest timestamp) to oldest (smallest timestamp) in the vector.
    //  If needed, access to `audioFeed` to render inference boxes.
    virtual bool process_spectrums(
        const std::vector<std::shared_ptr<AudioSpectrum>>& newSpectrums,
        AudioFeed& audioFeed
    ) = 0;

private:
    std::string m_label;
};




}
#endif
